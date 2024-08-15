Creating a map with gmapping:
```
export TURTLEBOT3_MODEL=waffle

roslaunch turtlebot3_gazebo turtlebot3_house.launch

roslaunch turtlebot3_slam turtlebot3_slam.launch slam_methods:=gmapping

rosrun map_server map_saver -f $(rospack find turtlebot3_navigation)/turtlebot3_house.yaml
```
System startup:
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
