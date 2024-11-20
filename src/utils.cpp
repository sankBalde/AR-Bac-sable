#include "utils.hpp"

#include <cmath>

std::vector<rgb8> get_cmap(float gamma) {
    std::vector<rgb8> color_map(2048);
    for (int i = 0; i < 2048; ++i) {
        float v = i / 2048.f;
		v = powf(v, gamma) * 6;
		int pval = v*6*256;

        int lb = pval & 0xff;
        rgb8 color = {0, 0, 0};
        switch (pval>>8) {
            case 0:
                color = {(uint8_t) 255, (uint8_t) (255-lb), (uint8_t) (255-lb)};
                break;
            case 1:
                color = {(uint8_t) 255, (uint8_t) lb, (uint8_t) 0};
                break;
            case 2:
                color = {(uint8_t) (255-lb), (uint8_t) 255, (uint8_t) 0};
                break;
            case 3:
                color = {(uint8_t) 0, (uint8_t) 255, (uint8_t) lb};
                break;
            case 4:
                color = {(uint8_t) 0, (uint8_t) (255-lb), (uint8_t) 255};
                break;
            case 5:
                color = {(uint8_t) 0, (uint8_t) 0, (uint8_t) (255-lb)};
                break;
            default:
                break;
        }
        color_map[i] = color;
    }
    return color_map;
}


// Génère la couleur en fonction de la hauteur normalisée
cv::Vec3b get_colormap_color(float height) {
    if (height <= -220.0f) return cv::Vec3b(0, 0, 0);           // Noir
    else if (height <= -200.0f) return cv::Vec3b(80, 0, 0);     // Rouge foncé
    else if (height <= -170.0f) return cv::Vec3b(100, 30, 0);   // Marron foncé
    else if (height <= -150.0f) return cv::Vec3b(102, 50, 0);   // Marron
    else if (height <= -125.0f) return cv::Vec3b(160, 108, 19); // Ocre foncé
    else if (height <= -7.5f) return cv::Vec3b(205, 140, 24);   // Ocre clair
    else if (height <= -2.5f) return cv::Vec3b(250, 206, 135);  // Beige clair
    else if (height <= -0.5f) return cv::Vec3b(255, 226, 176);  // Sable
    else if (height <= 0.0f) return cv::Vec3b(71, 97, 0);       // Vert foncé
    else if (height <= 5.0f) return cv::Vec3b(47, 122, 16);     // Vert herbe foncé
    else if (height <= 15.0f) return cv::Vec3b(60, 180, 40);    // Vert vif
    else if (height <= 25.0f) return cv::Vec3b(90, 220, 80);    // Vert clair
    else if (height <= 30.0f) return cv::Vec3b(240, 240, 60);   // Jaune clair
    else if (height <= 35.0f) return cv::Vec3b(255, 255, 160);  // Jaune sable
    else if (height <= 40.0f) return cv::Vec3b(255, 255, 255);  // Blanc
    else if (height <= 60.0f) return cv::Vec3b(0, 67, 161);     // Bleu profond
    else if (height <= 90.0f) return cv::Vec3b(30, 30, 130);    // Bleu foncé
    else if (height <= 140.0f) return cv::Vec3b(161, 161, 161); // Gris moyen
    else if (height <= 200.0f) return cv::Vec3b(206, 206, 206); // Gris clair
    else return cv::Vec3b(255, 255, 255);                       // Blanc
}




// Génère l'image colorisée de la profondeur
cv::Mat generate_colored_depth(const std::vector<uint16_t>& depth_vector, int width, int height, int min_depth, int max_depth) {
    cv::Mat depth_img(height, width, CV_8UC3);
    for (unsigned y = 0; y < height; ++y) {
        for (unsigned x = 0; x < width; ++x) {
            unsigned i = y * width + x;
            int nb = std::clamp(static_cast<int>(depth_vector[i]), min_depth, max_depth);
            float normalized_height = static_cast<float>(nb - min_depth) / (max_depth - min_depth) * (220.0f - -220.0f) + -220.0f;
            depth_img.at<cv::Vec3b>(y, x) = get_colormap_color(normalized_height);
        }
    }
    return depth_img;
}


// Ajoute des lignes de niveau avec des contours noirs
void add_contour_lines(cv::Mat& depth_img, const std::vector<uint16_t>& depth_vector, int width, int height, int step) {
    cv::Mat gray(height, width, CV_8UC1);

    // Créer une image en niveaux de gris à partir des valeurs de profondeur
    for (unsigned y = 0; y < height; ++y) {
        for (unsigned x = 0; x < width; ++x) {
            unsigned i = y * width + x;
            gray.at<uint8_t>(y, x) = static_cast<uint8_t>((depth_vector[i] % step) * 255 / step);
        }
    }

    // Détection des contours avec l'algorithme de Canny
    cv::Mat edges;
    cv::Canny(gray, edges, 50, 150);

    // Appliquer les contours noirs à l'image colorée
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (edges.at<uint8_t>(y, x) > 0) {
                depth_img.at<cv::Vec3b>(y, x) = cv::Vec3b(10, 10, 10); // Noir
            }
        }
    }
}


// Ajoute un ombrage pour simuler le relief
void add_shading(cv::Mat& depth_img, const cv::Mat& depth_map) {
    cv::Mat gradient_x, gradient_y;
    cv::Sobel(depth_map, gradient_x, CV_32F, 1, 0, 3);
    cv::Sobel(depth_map, gradient_y, CV_32F, 0, 1, 3);
    cv::Mat gradient_magnitude;
    cv::magnitude(gradient_x, gradient_y, gradient_magnitude);
    cv::normalize(gradient_magnitude, gradient_magnitude, 0, 255, cv::NORM_MINMAX);
    cv::Mat shading;
    gradient_magnitude.convertTo(shading, CV_8UC1);
    cv::applyColorMap(shading, shading, cv::COLORMAP_BONE);
    cv::addWeighted(depth_img, 0.7, shading, 0.3, 0, depth_img);
}

/*
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
}*/

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

// Fonction principale
uint8_t* process_depth(std::vector<uint16_t> depth_vector, int width, int height, int max_depth, int min_depth) {
    // Génération de l'image de profondeur colorisée
    cv::Mat depth_img = generate_colored_depth(depth_vector, width, height, min_depth, max_depth);

    // Ajout des effets
    add_contour_lines(depth_img, depth_vector, width, height, 100); // Lignes de niveau tous les 10 unités
    cv::Mat depth_map(height, width, CV_16UC1, depth_vector.data());
    add_shading(depth_img, depth_map);

    // Conversion en tableau d'octets
    uint8_t* res = new uint8_t[depth_img.total() * depth_img.elemSize()];
    std::memcpy(res, depth_img.data, depth_img.total() * depth_img.elemSize());
    return res;
}
