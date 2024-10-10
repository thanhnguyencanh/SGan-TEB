# Enhancing Social Robot Navigation with Integrated Motion Prediction and Trajectory Planning in Dynamic Human Environments
Implementation code for our paper ["Enhancing Social Robot Navigation with Integrated Motion Prediction and Trajectory Planning in Dynamic Human Environments"](https://)

Thanh Nguyen Canh, Xiem HoangVan, Nak Young Chong"**Enhancing Social Robot Navigation with Integrated Motion Prediction and Trajectory Planning in Dynamic Human Environments**, 2024. [[**IEEE Explore**](https://doi.org/)] [[Citation](#citation)]

## Citation
```
@INPROCEEDINGS{Canh2024Enhancing,
  title = {The International Conference on Control, Automation, and Systems (ICCAS)},
  DOI = {},
  booktitle = {Journal of Automation, Mobile Robotics and Intelligent Systems},
  publisher = {IEEE},
  author = {Thanh, Nguyen Canh and Xiem, HoangVan and NakYoung, Chong},
  year = {2024},
  pages = {}
}
```

## Our proposed:
<img src="https://github.com/thanhnguyencanh/SGan-TEB/blob/main/docs/overview.png" width="750px">
 
## Requirements
* ubuntu 20.04
* ROS Noetic


## Usage:

* 1. Step 1: Clone this repo
```
git clone https://github.com/thanhnguyencanh/SGan-TEB

catkin build
cd SGan-TEB
```
* 2. Step 2: Creating a map with mapping algorithm:
 
```
export TURTLEBOT3_MODEL=waffle

roslaunch turtlebot3_gazebo turtlebot3_house.launch

roslaunch turtlebot3_slam turtlebot3_slam.launch slam_methods:=gmapping

rosrun map_server map_saver -f $(rospack find turtlebot3_navigation)/turtlebot3_house.yaml
```

* 3. Step 3: System startup:


```
#Using Turtlebot 3 Waffle
export TURTLEBOT3_MODEL=waffle

#Initialize gazebo
roslaunch turtlebot3_gazebo turtlebot3_house.launch

#Run YOLO object detection
roslaunch darknet_ros darknet_ros.launch

#Run human trajectory tracking node
rosrun hdetect hdetect_node

#Run trajectory prediction node
rosrun sgan predict_tracks.py

#Navigate robot in Rviz
roslaunch turtlebot3_navigation turtlebot3_navigation.launch map_file:=$(rospack find turtlebot3_navigation)/turtlebot3_house.yaml
```
