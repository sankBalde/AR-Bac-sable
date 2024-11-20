#pragma once

#include <cstdint>
#include <vector>
#include <opencv2/imgproc.hpp>

struct rgb8
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

std::vector<rgb8> get_cmap(float gamma = 3.f);

uint8_t* process_depth(std::vector<uint16_t> depth_vector, int width, int height, int max_depth, int min_depth);
cv::Mat uint8ArrayToMat(uint8_t* data, int rows, int cols, int type);
std::vector<uint16_t> matToVector(cv::Mat_<uint16_t>& mat);

