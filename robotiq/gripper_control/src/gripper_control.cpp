#include "ros/ros.h"
#include "std_msgs/String.h"
#include "robotiq_c_model_control/CModel_robot_input.h"
#include "robotiq_c_model_control/CModel_robot_output.h"
#include "gripper_srv/Grasp.h"
#include <sstream>

#define unsigned char uint8

using namespace std;

bool executed = 0;
bool in_goal = 0;
int current_position;
int command_position;
int feedback_current;
robotiq_c_model_control::CModel_robot_input input;
robotiq_c_model_control::CModel_robot_output output;

class gripper_pub_sub
{
public:
  ros::NodeHandle nh_;
//  ros::Publisher test_gripper_pub;
  ros::Subscriber test_gripper_sub;

  gripper_pub_sub()
  {
//    test_gripper_pub = nh_.advertise<robotiq_c_model_control::CModel_robot_output>("/CModelRobotOutput", 1);
    test_gripper_sub = nh_.subscribe("/CModelRobotInput", 1, &gripper_pub_sub::gripperCallback,this);

  }

  ~gripper_pub_sub()
  {
  }

  void gripperCallback(const robotiq_c_model_control::CModel_robot_input::ConstPtr& msg)
  {
      // cout<<"Obj is "<<(int)(msg->gOBJ)<<endl;
      if(msg->gOBJ > 0 && !executed)
      {
          executed = true;
          if((int)(msg->gOBJ) == 3)
              in_goal = 1;
          else
              in_goal = 0;
          current_position = (int)msg->gPO;
          command_position = (int)msg->gPR;
          feedback_current = (int)msg->gCU;
      }

//      msg->gACT;    // active
//      msg->gGTO;    // get command
//      msg->gSTA;    // actived
//      msg->gOBJ;    // reach target position
//      msg->gFLT;    // is default state?
//      msg->gPR;     // command position
//      msg->gPO;     // current position
//      msg->gCU;     // current(mA)
  }

};

bool conditionCB(gripper_srv::Grasp::Request &req,gripper_srv::Grasp::Response &res)
{
    ros::NodeHandle nh_new;
    ros::Publisher test_gripper_pub = nh_new.advertise<robotiq_c_model_control::CModel_robot_output>("/CModelRobotOutput", 1);
    robotiq_c_model_control::CModel_robot_output msg;
    msg.rACT = 1;
    msg.rGTO = 1;
    msg.rATR = 0;
    msg.rPR = req.position;
    msg.rSP = req.speed;
    msg.rFR = req.force;
    test_gripper_pub.publish(msg);
    executed = false;
    sleep(1.5);
    ROS_INFO("success");

    while(!executed)
    {
        ros::spinOnce();
    }
    res.in_goal = in_goal;
    res.current_position = current_position;
    res.command_position = command_position;
    res.feedback_current = feedback_current;
    executed = false;
    return true;
}


int main(int argc, char** argv)
{
    ros::init(argc, argv, "gripper_control");
    ros::NodeHandle n;
    ros::Rate loop_rate(20);

    ros::Publisher enable_gripper_pub = n.advertise<robotiq_c_model_control::CModel_robot_output>("/CModelRobotOutput", 1);
    robotiq_c_model_control::CModel_robot_output enable_msg;
    // de-active
    enable_gripper_pub.publish(enable_msg);
    // active
    enable_msg.rACT = 1;		// 激活
    enable_msg.rGTO = 0;		// 使能
    enable_msg.rATR = 0;		// TODO:不知道
    enable_msg.rPR = 0;		// position
    enable_msg.rSP = 0;		// speed
    enable_msg.rFR = 0;		// force
    sleep(1);
    enable_gripper_pub.publish(enable_msg);
    sleep(1);

    ros::ServiceServer service = n.advertiseService("gripper_service", conditionCB);
    gripper_pub_sub g1;
    ROS_INFO("********************************");
    ROS_INFO("You can use robotiq gripper now!");


    while (ros::ok())
    {
        ros::spinOnce();
        loop_rate.sleep();
    }
    ros::spin();
    return 0;
}
