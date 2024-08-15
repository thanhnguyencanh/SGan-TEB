# Enhancing Social Robot Navigation with Integrated Motion Prediction and Trajectory Planning in Dynamic Human Environments
Implementation code for our paper ["M-Calib: A Monocular 3D Object Localization using 2D Estimates for Industrial Robot Vision System"](https://assets.researchsquare.com/files/rs-4019542/v1_covered_5a75ac68-1bc8-4bdd-b2c5-8bbdb1eac8f1.pdf?c=1711473654)

Thanh Nguyen Canh, Du Trinh Ngoc, Xiem HoangVan, "**Monocular 3D Object Localization using 2D Estimates for Industrial Robot Vision System**," *Journal of Automation, Mobile Robotics and Intelligent Systems*, 2024. [[**Journal of Automation, Mobile Robotics and Intelligent Systems**](https://doi.org/)] [[Citation](#citation)]

## Citation
```
@article{Canh2024,
  title = {Monocular 3D Object Localization using 2D Estimates for Industrial Robot Vision System},
  ISSN = {},
  url = {},
  DOI = {},
  journal = {Journal of Automation, Mobile Robotics and Intelligent Systems},
  publisher = {Industrial Research Institute for Automation and Measurements PIAP, Poland},
  author = {Thanh, Nguyen Canh and Du, Trinh Ngoc and Xiem HoangVan},
  year = {2024},
  month = jul,
  pages = {}
}
```

## Our proposed:
<img src="https://github.com/thanhnguyencanh/SGan-TEB/blob/main/docs/overview.png" width="750px">
 
## Requirements
* python 3.7
* torch 1.7.1
* tensorboard

## M-Calib Datasets
There are two different datasets collected by the authors

The related datasets can be found at:

* 1. Object Detection dataset: (https://app.roboflow.com/uet-jvl1l/m-calib/1).
* 2. Object Segmentation dataset: (https://app.roboflow.com/uet-jvl1l/mcalibsegment/1).

## Usage: M-Calib (the inference)

* 1. Step 1: Clone this repo
```
git clone https://github.com/thanhnguyencanh/MonoCalibNet
cd MonoCalibNet
```
* 2. Step 2: Creating a model

Option 1: 
+ Use a pre-trained model: [model](https://drive.google.com/drive/folders/1MS6DLxgKxo-FtC7TSTJN8WJCxhu8W3Fe?usp=sharing)

+ Modify path in [cfg.py](https://github.com/thanhnguyencanh/MonoCalibNet/blob/main/cfg.py)

Option 2: 
+ Training new model using [Object_Detection](https://github.com/thanhnguyencanh/MonoCalibNet/blob/main/Object_Detection.ipynb) and [Instance_Segmentation](https://github.com/thanhnguyencanh/MonoCalibNet/blob/main/Instance_Segmentation.ipynb) 

+ Convert to Onnx model

* 3. Step 3: Testing

```
mkdir model
mkdir dataset
```
 + Set pat in cfg.py
```
python3 run_chessboard_yolov5.py
```


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
