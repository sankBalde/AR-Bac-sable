#include "calibrate-qt.hpp"
#include <opencv2/imgproc.hpp>  // Pour cv::cvtColor et cv::COLOR_BGR2RGB
#include "utils.hpp"


uint8_t* process_depth(std::vector<uint16_t> depth_vector, int width, int height, int max_depth, int min_depth) {
    // Lissage de l'image de profondeur avec un filtre moyen


    // Création de l'image de profondeur en couleur
    cv::Mat depth_img(height, width, CV_8UC3);

    // Calcul des valeurs min et max
    uint16_t actual_min = *std::min_element(depth_vector.begin(), depth_vector.end());
    uint16_t actual_max = *std::max_element(depth_vector.begin(), depth_vector.end());

    // Clamp les valeurs pour éviter les valeurs extrêmes
    if (actual_max == actual_min) {
        actual_max += 1; // Évite la division par zéro
    }

    for (unsigned y = 0; y < height; ++y) {
        for (unsigned x = 0; x < width; ++x) {
            unsigned i = y * width + x;

            // Clamp les valeurs de profondeur dans la plage de min_depth à max_depth
            int nb = std::clamp(static_cast<int>(depth_vector[i]), min_depth, max_depth);
            cv::Vec3b& color = depth_img.at<cv::Vec3b>(y, x);

            // Normalisation de nb pour le mapping de couleur
            float normalized_value = static_cast<float>(nb - min_depth) / (max_depth - min_depth) * 255;

            // Clamping supplémentaire pour éviter les valeurs extrêmes dans la normalisation
            normalized_value = std::clamp(normalized_value, 0.0f, 255.0f);

            // Application de la palette de couleurs topographiques inversée
            if (normalized_value <= 40) {
                color = cv::Vec3b(255, 0, 0);   // Sommets (rouge)
            } else if (normalized_value > 40 && normalized_value <= 80) {
                color = cv::Vec3b(255, 128, 0); // Hautes montagnes (orange)
            } else if (normalized_value > 80 && normalized_value <= 100) {
                color = cv::Vec3b(255, 255, 0); // Montagnes moyennes (jaune)
            } else if (normalized_value > 100 && normalized_value <= 120) {
                color = cv::Vec3b(0, 255, 0);   // Collines (vert clair)
            } else if (normalized_value > 120 && normalized_value <= 150) {
                color = cv::Vec3b(0, 128, 0);   // Plaines (vert)
            } else if (normalized_value > 150 && normalized_value <= 200) {
                color = cv::Vec3b(0, 64, 255);  // Eau (bleu clair)
            } else  {
                color = cv::Vec3b(0, 0, 128);   // Océan profond (bleu foncé)
            }
        }
    }

    // Conversion de l'image en tableau d'octets pour le retour
    uint8_t* res = new uint8_t[depth_img.total() * depth_img.elemSize()];
    std::memcpy(res, depth_img.data, depth_img.total() * depth_img.elemSize());
    return res;
}

std::vector<uint16_t> matToVector(cv::Mat_<uint16_t>& mat) {

    if (mat.empty()) {
        return std::vector<uint16_t>();
    }


    std::vector<uint16_t> vec(mat.begin(), mat.end());

    return vec;
}

cv::Mat uint8ArrayToMat(uint8_t* data, int rows, int cols, int type) {

    return cv::Mat(rows, cols, type, data).clone();
}

cv::Mat depthmap_colorize(cv::Mat _depth, int min_depth, int max_depth)
{

    auto now = std::chrono::high_resolution_clock::now();

    static auto cmap = get_cmap(4.f);
    cv::Mat_<uint16_t> depth16 = _depth;

    std::vector<uint16_t> depth_vector = matToVector(depth16);
    uint8_t* final_img = process_depth(depth_vector,depth16.cols, depth16.rows, max_depth, min_depth);
    cv::Mat depth_rgb2 = uint8ArrayToMat(final_img, depth16.rows, depth16.cols, CV_8UC3);
    cv::Mat rgbImage;
    cv::cvtColor(depth_rgb2, rgbImage, cv::COLOR_BGR2RGB);
    return depth_rgb2;
}

/*
cv::Mat depthmap_colorize(cv::Mat _depth, int min_depth, int max_depth)
{
    static auto cmap = get_cmap(5.f);
    cv::Mat_<uint16_t> depth16 = _depth;

    // Scale the depth image
    if (min_depth > 0 && max_depth > 0)
    {
        min_depth = std::clamp((int)(0.75f * min_depth), 0, 2047);
        max_depth = std::clamp((int)(1.25f * max_depth), 0, 2047);
        depth16.forEach([&](uint16_t &pixel, const int position[2]) {
            int value = (pixel - min_depth) * 2048 / (max_depth - min_depth);
            pixel = std::clamp(value, 0, 2047);
        });
    }


    // Colorize the unwrapped depth image
    cv::Mat depth_rgb(depth16.rows, depth16.cols, CV_8UC3);
    depth16.forEach([&](uint16_t &pixel, const int position[2]) {
        rgb8 color = cmap[pixel];
        depth_rgb.at<cv::Vec3b>(position[0], position[1]) = cv::Vec3b(color.r, color.g, color.b); }
    );

    return depth_rgb;
}*/

int main(int argc, char** argv)
{
    // Create a QT application with a window and side-by-side RGB and Depth panel
    QApplication app(argc, argv);

    QCalibrationApp win;
    win.setOnDepthFrameChange(depthmap_colorize);
    win.show();

    return app.exec();
}