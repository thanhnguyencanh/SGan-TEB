// #include "ros/ros.h"
// #include <cmath>

// #include <memory>
// #include <limits>
// #include "ros/time.h" 
// #include "std_msgs/Time.h"
// #include <nav_msgs/Path.h>
// #include <geometry_msgs/PoseStamped.h>
// #include <tf/transform_datatypes.h>


// #include <nav_msgs/Odometry.h>
// #include <sensor_msgs/LaserScan.h>

// #include <geometry_msgs/PoseWithCovarianceStamped.h>

// int main(int argc, char **argv)
// {
//     // Khởi tạo node
//     ros::init(argc, argv, "my_node");

//     // Tạo một handle cho node
//     ros::NodeHandle n;

//     // In ra một thông điệp
//     ROS_INFO("Hello, ROS!");

//     // Chạy vòng lặp ROS
//     ros::spin();

//     return 0;
// }
#include <cmath>

#include <memory>
#include <limits>
#include "ros/time.h" 
#include "std_msgs/Time.h"
#include "ros/ros.h"
#include <nav_msgs/Path.h>
#include <geometry_msgs/PoseStamped.h>
#include <tf/transform_datatypes.h>


#include <nav_msgs/Odometry.h>
#include <sensor_msgs/LaserScan.h>

#include <geometry_msgs/PoseWithCovarianceStamped.h>
#include <geometry_msgs/PoseArray.h>
#include <track_msg/Track.h>

// #include <spencer_tracking_msgs/TrackedPersons.h>
// #include <visualization_msgs/Marker.h>
// #include <visualization_msgs/MarkerArray.h>
struct Point {
    double x;
    double y;
    double z;
};
int signalo=0;
int MAX_PEOPLE=0;
struct Point humanArray[10][20];
geometry_msgs::PoseWithCovarianceStamped robot_pose;
int humanCount[10];

ros::Subscriber _detectionSub, amcl_pose_sub;
ros::Publisher _detectionPub;

void amclPoseCallback(const geometry_msgs::PoseWithCovarianceStamped::ConstPtr& amcl_pose_msg)
{
  // printf("1\n");
  robot_pose = *amcl_pose_msg;
  robot_pose.pose.pose.orientation.z+=4.71;
}

void humanDetectCallback(const geometry_msgs::PoseArray &msg)
{
  struct Point humanArray2[10][20];
  
  if(msg.poses.size() > 0){
    for (int i = 0; i < 10; ++i) {
      for (int j = 0; j < 20; ++j) {
          humanArray2[i][j].x = 1000;
          humanArray2[i][j].y = 1000;
      }
    }
    for (int i = 0; i < 10; ++i) {
      for (int j = 0; j < 20; ++j) {
          humanArray2[i][j].x = humanArray[i][j].x;
          humanArray2[i][j].y = humanArray[i][j].y;
      }
    }
    for (int i = 0; i < 10; ++i) {
      for (int j = 0; j < 20; ++j) {
          humanArray[i][j].x = 1000;
          humanArray[i][j].y = 1000;
      }
    }
  }

  double R[2][2] = {
      {cos(robot_pose.pose.pose.orientation.z), -sin(robot_pose.pose.pose.orientation.z)},
      {sin(robot_pose.pose.pose.orientation.z),  cos(robot_pose.pose.pose.orientation.z)}
  };
  // double human_x_real = robot_pose.pose.pose.position.x + R[0][0] * msg.poses[0].position.x + R[0][1] * msg.poses[0].position.y;
  // double human_y_real = robot_pose.pose.pose.position.y + R[1][0] * msg.poses[0].position.x + R[1][1] * msg.poses[0].position.y;
  // double human_x_real = msg.poses[0].position.x * cos(robot_pose.pose.pose.orientation.z) - msg.poses[0].position.y * sin(robot_pose.pose.pose.orientation.z) + robot_pose.pose.pose.position.x;
  // double human_y_real = msg.poses[0].position.x * sin(robot_pose.pose.pose.orientation.z) + msg.poses[0].position.y * cos(robot_pose.pose.pose.orientation.z) + robot_pose.pose.pose.position.y;
  // printf("a%d\n",MAX_PEOPLE);
  // printf("r%.2f %.2f\n",robot_pose.pose.pose.position.x,robot_pose.pose.pose.position.y);
  // printf("h%.2f %.2f\n",msg.poses[0].position.x,msg.poses[0].position.y);
  // printf("hr%.2f %.2f\n",human_x_real,human_y_real);
  // printf("z%.2f\n",robot_pose.pose.pose.orientation.z);
  int dtemp=0;
  int MAX_PEOPLE_OR = MAX_PEOPLE;
  // printf("%d\n",dtemp);
  for (size_t i = 0; i < msg.poses.size(); ++i){
    int d=0, sp=0;
    double min=1000;
    double human_x_real = robot_pose.pose.pose.position.x + R[0][0] * msg.poses[i].position.x + R[0][1] * msg.poses[i].position.y;
    double human_y_real = robot_pose.pose.pose.position.y + R[1][0] * msg.poses[i].position.x + R[1][1] * msg.poses[i].position.y;
    for (int m = 0; m < MAX_PEOPLE_OR; ++m) {
      double dis = sqrt((human_x_real-humanArray2[m][humanCount[m]-1].x)*(human_x_real-humanArray2[m][humanCount[m]-1].x)+(human_y_real-humanArray2[m][humanCount[m]-1].y)*(human_y_real-humanArray2[m][humanCount[m]-1].y));
      if(humanCount[m] == 8){
        if((min > dis)&&(dis<1)){
          min = dis;
          for (int k = 0; k < 7; ++k) {
            humanArray2[m][k].x = humanArray2[m][k+1].x;
            humanArray2[m][k].y = humanArray2[m][k+1].y;
          }
          humanArray2[m][7].x = human_x_real;
          humanArray2[m][7].y = human_y_real;
          sp = m;
          d=1;
        }
      }else{
        if(min > dis){
          min = dis;
          humanArray2[m][humanCount[m]].x = human_x_real;
          humanArray2[m][humanCount[m]].y = human_y_real;
          sp = m;
          d=1;
        }
      }
    }
    for (int i = 0; i < 10; ++i) {
      humanCount[i] = 0;
    }
    if(d==1){
      for (int j = 0; j < 8; ++j) {
        humanArray[dtemp][j].x = humanArray2[sp][j].x;
        humanArray[dtemp][j].y = humanArray2[sp][j].y;
      }
      if (humanCount[sp] < 8) humanCount[sp]++;
      dtemp++;
      MAX_PEOPLE = dtemp;
    } else{
      humanArray[dtemp][0].x = human_x_real;
      humanArray[dtemp][0].y = human_y_real;
      humanCount[dtemp] = 1;
      dtemp++;
      MAX_PEOPLE = dtemp;
    }

    // printf("a%d\n",MAX_PEOPLE);
    
    
  }
  
  track_msg::Track msg2;
  for (int i = 0; i < MAX_PEOPLE; ++i) {
      // printf("a%d b%d\n",i,humanCount[i]);
      geometry_msgs::PoseArray p2;
      for (int j = 0; j < 8 - humanCount[i]; ++j) {
          // printf("%.2f %.2f\n", humanArray[i][0].x, humanArray[i][0].y);
          geometry_msgs::Pose p;
          p.position.x = humanArray[i][0].x;
          p.position.y = humanArray[i][0].y;
          p2.poses.push_back(p);
      }
      for (int j = 8 - humanCount[i]; j < 8; ++j) {
          // printf("%.2f %.2f\n", humanArray[i][j - 8 + humanCount[i]].x, humanArray[i][j - 8 + humanCount[i]].y);
          geometry_msgs::Pose p;
          p.position.x = humanArray[i][j - 8 + humanCount[i]].x;
          p.position.y = humanArray[i][j - 8 + humanCount[i]].y;
          p2.poses.push_back(p);
      }
      // for (int j = 0; j < 8; ++j) {
          
      //     geometry_msgs::Pose p;
      //     p.position.x = humanArray[i][j].x;
      //     p.position.y = humanArray[i][j].y;
      //     p2.poses.push_back(p);
      // }
      msg2.tracks.push_back(p2);
      
  }
  _detectionPub.publish(msg2);
  
/*
  //     // printf("%d\n",dtemp);
  //     for (int n = 0; n < 8; ++n) {
  //       // printf("m%d\n",m);
  //       // printf("n%d\n",n);
  //       if(n==7){
  //         if ((sqrt((human_x_real-humanArray2[m][n].x)*(human_x_real-humanArray2[m][n].x)+(human_y_real-humanArray2[m][n].y)*(human_y_real-humanArray2[m][n].y))<0.2)){
  //           for (int k = 0; k < 7; ++k) {
  //             humanArray2[m][k].x = humanArray2[m][k+1].x;
  //             humanArray2[m][k].y = humanArray2[m][k+1].y;
  //           }
  //           humanArray2[m][7].x = human_x_real;
  //           humanArray2[m][7].y = human_y_real;
  //           humanCount[m]=8;
  //           d=1;
  //           dd = 1;
  //           n=8;
  //         } else{
  //           if (abs(humanArray2[m][n].x - 1000) < 0.001){
  //             if ((sqrt((human_x_real-humanArray2[m][n-1].x)*(human_x_real-humanArray2[m][n-1].x)+(human_y_real-humanArray2[m][n-1].y)*(human_y_real-humanArray2[m][n-1].y))<0.2)){
  //               humanArray2[m][n].x = human_x_real;
  //               humanArray2[m][n].y = human_y_real;
  //               humanCount[m]=n+1;
  //               d=1;
  //               dd = 1;
  //               n=8;
  //             }
  //           }
  //         }
  //       }
  //       if ((n>0)&&(n<7)){
  //         if (abs(humanArray2[m][n].x - 1000) < 0.001){
  //           if ((sqrt((human_x_real-humanArray2[m][n-1].x)*(human_x_real-humanArray2[m][n-1].x)+(human_y_real-humanArray2[m][n-1].y)*(human_y_real-humanArray2[m][n-1].y))<0.2)){
  //             humanArray2[m][n].x = human_x_real;
  //             humanArray2[m][n].y = human_y_real;
  //             humanCount[m]=n+1;
  //             d=1;
  //             dd = 1;
  //             n=8;
  //           }
  //         }
  //       }
  //     }
  //     if (d==1){
  //       for (int j = 0; j < 8; ++j) {
  //           humanArray[dtemp][j].x = humanArray2[m][j].x;
  //           humanArray[dtemp][j].y = humanArray2[m][j].y;
  //       }
  //       dtemp++;
  //       MAX_PEOPLE = dtemp;
  //       // printf("1%d\n",MAX_PEOPLE);
  //       // for (int j = 0; j < 8; ++j) {
  //       //     printf("%.2f %.2f\n",humanArray[dtemp-1][j].x,humanArray[dtemp-1][j].y); 
  //       // }
        
  //     }
  //   }
  //   if (dd==0){
  //     humanArray[dtemp][0].x = human_x_real;
  //     humanArray[dtemp][0].y = human_y_real;
  //     dtemp++;
  //     // printf("%d %d\n",MAX_PEOPLE,dtemp);
  //     MAX_PEOPLE = dtemp;
  //     // printf("2%d\n",MAX_PEOPLE);
  //     // for (int j = 0; j < 8; ++j) {
  //     //     printf("%.2f %.2f\n",humanArray[dtemp-1][j].x,humanArray[dtemp-1][j].y); 
  //     // }
  //   }
  // }
*/
}

// void publishMsg2(const ros::TimerEvent& event)
// {
//   _detectionPub.publish(msg2);
// }
    
int main(int argc, char **argv)
{
    // Khởi tạo node
    ros::init(argc, argv, "hdetect");
    ros::NodeHandle nh;

    // Tạo một handle cho node
    
    for (int i = 0; i < 10; ++i) {
      for (int j = 0; j < 20; ++j) {
          humanArray[i][j].x = 1000;
          humanArray[i][j].y = 1000;
      }
    }
    for (int i = 0; i < 10; ++i) {
      humanCount[i]=0;
    }

    _detectionSub = nh.subscribe("/darknet_ros/detections", 1, humanDetectCallback);

    _detectionPub = nh.advertise<track_msg::Track>("/tracks", 1);

    // ros::Timer timer = nh.createTimer(ros::Duration(0.01), publishMsg2);


    amcl_pose_sub = nh.subscribe<geometry_msgs::PoseWithCovarianceStamped>("/amcl_pose", 1, amclPoseCallback);

    // Chạy vòng lặp ROS
    ros::spin();
    // ros::Rate rate(100); // 100Hz, tương đương 0.01s
    // while (ros::ok())
    // {
    //     _detectionPub.publish(msg2);
    //     ros::spinOnce();
    //     rate.sleep();
    // }

    return 0;
}