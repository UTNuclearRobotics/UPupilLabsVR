// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Chifor Tudor
// Updated by: Christina Petlowany, The University of Texas at Austin

#pragma once

// #define   MSGPACK_USE_CPP03

#undef check
#include "msgpack.hpp"
#include "msgpack/adaptor/define_decl.hpp"

struct eye_center_3d {
	float x;
	float y;
	float z;
	MSGPACK_DEFINE_ARRAY(x, y, z);
};

struct gaze_normal_3d {
	float x;
	float y;
	float z;
	MSGPACK_DEFINE_ARRAY(x, y, z);
};

struct vector_3d {
	float x;
	float y;
	float z;
	MSGPACK_DEFINE_ARRAY(x, y, z);
};

struct norm_pos {
	float x;
	float y;
	MSGPACK_DEFINE_ARRAY(x, y);
};

struct axes {
	float x;
	float y;
	MSGPACK_DEFINE_ARRAY(x, y);
};

struct center {
	float x;
	float y;
	MSGPACK_DEFINE_ARRAY(x, y);
};

struct ellipse {
	center center;
	axes axes;
	float angle;
	MSGPACK_DEFINE_MAP(center, axes, angle);
};

struct pupil {
	std::string topic;
	float confidence;
	ellipse ellipse;
	float diameter;
	norm_pos norm_pos;
	float timestamp;
	std::string method;
	float id;
	MSGPACK_DEFINE_MAP(topic, confidence, ellipse, diameter, norm_pos, timestamp, method, id);
};

struct base_data {
	pupil pupil1;
	pupil pupil2;
	MSGPACK_DEFINE_ARRAY(pupil1, pupil2);
};

struct GazeStruct {
	eye_center_3d eye_center_3d;
	gaze_normal_3d gaze_normal_3d;
	std::map<int, vector_3d> eye_centers_3d;
	std::map<int, vector_3d> gaze_normals_3d;
	std::string topic;
	norm_pos norm_pos;
	base_data base_data;
	float confidence;
	float id;
	float timestamp;
	MSGPACK_DEFINE_MAP(eye_center_3d, gaze_normal_3d, eye_centers_3d, gaze_normals_3d, topic, norm_pos, confidence, id, timestamp, base_data);
};

//Todo move in struct folder

struct DetectionModeStruct
{
	std::string subject;
	std::string mode;
	MSGPACK_DEFINE_MAP(subject, mode);

};

struct EyeStruct
{
	std::string subject;
	int eye_id;
	float delay;
	MSGPACK_DEFINE_MAP(subject, eye_id, delay);

};

struct StartPluginStruct
{
	std::string subject;
	std::string name;
	MSGPACK_DEFINE_MAP(subject, name);
};

struct CalibrationShouldStartStruct
{
	std::string subject;
	float hmd_video_frame_size[2];
	int outlier_threshold;
	float translation_eye0[3]; //Like an FVector
	float translation_eye1[3];
	MSGPACK_DEFINE_MAP(subject, hmd_video_frame_size, outlier_threshold, translation_eye0, translation_eye1);

};

struct CalibrationShouldStopStruct
{
	std::string subject;
	MSGPACK_DEFINE_MAP(subject);

};

/**We use a Vector of structs to represent most values and this is just one element of it*/
struct AddCalibrationReferenceElementStruct
{
	norm_pos norm_pos;
	float timestamp;
	int id;
	MSGPACK_DEFINE_ARRAY(norm_pos, timestamp, id);
};

struct AddCalibrationReferenceDataStruct
{
	std::string subject;
	AddCalibrationReferenceElementStruct ref_data[218];
	MSGPACK_DEFINE_MAP(subject, ref_data);
};