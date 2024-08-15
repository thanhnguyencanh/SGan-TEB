#!/usr/bin/env python

import numpy as np
import torch
import os
from collections import deque, OrderedDict
from copy import copy
from attrdict import AttrDict
import rospy
import threading
import time

from models import TrajectoryGenerator
from utils import relative_to_abs

from geometry_msgs.msg import Pose,PoseArray
from track_msg.msg import Track
from visualization_msgs.msg import Marker, MarkerArray

class SGANPredictor:
    def __init__(self):
        model_path = "models/sgan-p-models/"  
        model_name = "hotel_12"  
        model = model_name + "_model.pt"
        # checkpoint = torch.load(os.path.join(model_path, model), map_location=torch.device('cpu'))
        checkpoint = torch.load("/home/adminok/catkinws/src/sgan/scripts/hotel_12_model.pt", map_location=torch.device('cpu'))
        self.num_samples = 20
        self.seq_len = 8
        self.visualise = True
        self.args_ = AttrDict(checkpoint['args'])
        self.generator = self.get_generator(checkpoint)
        self.tracked_persons_sub = rospy.Subscriber("/tracks", Track, self.predict_tracks, queue_size=3)
        self.predictions_marker_pub = rospy.Publisher("/predictors", Track, queue_size=1)
        self.tracked_persons = {}
        self.msg2 = Track()

    def get_generator(self, checkpoint):
        generator = TrajectoryGenerator(
            obs_len=self.args_.obs_len,
            pred_len=self.args_.pred_len,
            embedding_dim=self.args_.embedding_dim,
            encoder_h_dim=self.args_.encoder_h_dim_g,
            decoder_h_dim=self.args_.decoder_h_dim_g,
            mlp_dim=self.args_.mlp_dim,
            num_layers=self.args_.num_layers,
            noise_dim=self.args_.noise_dim,
            noise_type=self.args_.noise_type,
            noise_mix_type=self.args_.noise_mix_type,
            pooling_type=self.args_.pooling_type,
            pool_every_timestep=self.args_.pool_every_timestep,
            dropout=self.args_.dropout,
            bottleneck_dim=self.args_.bottleneck_dim,
            neighborhood_size=self.args_.neighborhood_size,
            grid_size=self.args_.grid_size,
            batch_norm=self.args_.batch_norm)
        generator.load_state_dict(checkpoint['g_state'])
        generator.train()
        return generator
    
    def publish_msg2(self):
        while True:
            self.predictions_marker_pub.publish(self.msg2)
            time.sleep(0.01)

    # Start the thread

    def visualise_predictions(self, pred_samples):
        # # print("Predicted Trajectories:")
        # for track_id, traj in pred_samples.items():
        #     # print(f"Track ID: {track_id}, Trajectory: {traj}\n")
        #     msg=Track()
        #     for j in range(min(12, len(traj))):  
        #         msg.tracks[track_id].poses[j].position.x = traj[j][0]
        #         msg.tracks[track_id].poses[j].position.y = traj[j][1]
        # self.predictions_marker_pub.publish(msg)

        # print("Predicted Trajectories:")
        
        self.msg2 = Track()
        
        for track_id, traj in pred_samples.items():
            msgPoseArray = PoseArray()  
            for j in range(12):  
                    msgPose = Pose()
                    msgPose.position.x =   traj[j][0]
                    msgPose.position.y =   traj[j][1]
                    msgPoseArray.poses.append(copy(msgPose))
                    # msg.tracks[track_id].poses[j].position.x = traj[j][0]
                    # msg.tracks[track_id].poses[j].position.y = traj[j][1]
            self.msg2.tracks.append(copy(msgPoseArray))
        # self.predictions_marker_pub.publish(self.msg2)


    def predict_tracks(self,msg):
        # self.tracked_persons = []
        # for i in range(len(msg.tracks)): 
        #     for j in range(8): 
        #         self.tracked_persons[i][j][0] = msg.tracks[i].poses[j].position.x

        self.tracked_persons = {}

        for track_id, track in enumerate(msg.tracks):
            current_track = []
            for pose in track.poses:
                current_pose = [pose.position.x, pose.position.y]
                current_track.append(current_pose)
            self.tracked_persons[track_id] = current_track

        # self.tracked_persons[0] = [
        #     [17.5     ,   10.938     ],
        #     [ 17.25     ,   10.938     ],
        #     [ 17     ,   10.938     ],
        #     [ 16.75     ,   10.958     ],
        #     [ 16.5     ,   10.998     ],
        #     [ 16.25     ,   10.996     ],
        #     [ 16     ,   10.992     ],
        #     [ 15.75     ,   10.861     ],
        # ]
        # self.tracked_persons[1] = [
        #     [10     ,   5.938     ],
        #     [ 10.21     ,   5.938     ],
        #     [ 10.44   ,   5.938     ],
        #     [ 10.6     ,   5.958     ],
        #     [ 10.79     ,   5.998     ],
        #     [ 11.05     ,   5.996     ],
        #     [ 11.19     ,   5.992     ],
        #     [ 11.4     ,   5.861     ]
        # ]
        curr_seq = np.zeros((self.seq_len, len(self.tracked_persons), 2))
        curr_seq_rel = np.zeros(curr_seq.shape)
        valid_tracks = 0

        with torch.no_grad():
            for id, tracks in self.tracked_persons.items():
                # if len(tracks) != self.seq_len:
                #     continue
                obs_traj = np.array([[track[0], track[1]] for track in tracks])
                obs_traj_rel = np.zeros(obs_traj.shape)
                obs_traj_rel[1:, :] = obs_traj[1:, :] - obs_traj[:-1, :]
                curr_seq[:, valid_tracks, :] = obs_traj
                curr_seq_rel[:, valid_tracks, :] = obs_traj_rel
                valid_tracks += 1

            if valid_tracks > 0:
                curr_seq = torch.from_numpy(curr_seq[:, :valid_tracks, :]).float()
                curr_seq_rel = torch.from_numpy(curr_seq_rel[:, :valid_tracks, :]).float()
                l = [valid_tracks]
                cum_start_idx = [0] + np.cumsum(l).tolist()
                seq_start_end = [
                    (start, end)
                    for start, end in zip(cum_start_idx, cum_start_idx[1:])
                ]
                seq_start_end = torch.Tensor(seq_start_end).type(torch.int)
                pred_samples = {track_id: [] for track_id in self.tracked_persons.keys()}
                track_ids = list(self.tracked_persons.keys())
                for _ in range(self.num_samples):
                    pred_traj_fake_rel = self.generator(curr_seq, curr_seq_rel, seq_start_end)
                    pred_traj_fake = relative_to_abs(pred_traj_fake_rel, curr_seq[-1])
                    for ped_id, pred in enumerate(pred_traj_fake.transpose(1, 0).tolist()):
                        # pred_samples[track_ids[ped_id]].extend(pred)
                        if len(pred_samples[track_ids[ped_id]]) < 12:
                            remaining_slots = 12 - len(pred_samples[track_ids[ped_id]])
                            pred_samples[track_ids[ped_id]].extend(pred[:remaining_slots])

                if self.visualise:
                    self.visualise_predictions(pred_samples)

    # def run(self):
    #     # for i in range(num_tracks):

    #     self.tracked_persons[0] = [
    #         [17.5     ,   10.938     ],
    #         [ 17.25     ,   10.938     ],
    #         [ 17     ,   10.938     ],
    #         [ 16.75     ,   10.958     ],
    #         [ 16.5     ,   10.998     ],
    #         [ 16.25     ,   10.996     ],
    #         [ 16     ,   10.992     ],
    #         [ 15.75     ,   10.861     ],
    #     ]
    #     self.tracked_persons[1] = [
    #         [10     ,   5.938     ],
    #         [ 10.21     ,   5.938     ],
    #         [ 10.44   ,   5.938     ],
    #         [ 10.6     ,   5.958     ],
    #         [ 10.79     ,   5.998     ],
    #         [ 11.05     ,   5.996     ],
    #         [ 11.19     ,   5.992     ],
    #         [ 11.4     ,   5.861     ]
    #     ]

    #     self.predict_tracks()

if __name__ == "__main__":
    # sgan_predictor = SGANPredictor()
    # sgan_predictor.run()
    rospy.init_node("sgan_predictor")
    sgan_node = SGANPredictor()
    threading.Thread(target=sgan_node.publish_msg2).start()
    rospy.spin()
