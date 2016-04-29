#ifndef CPV_RANDOM_DECISION_FOREST
#define CPV_RANDOM_DECISION_FOREST

#include "include/RandomDesicionTree.h"


class RandomDecisionForest : public QObject
{
    Q_OBJECT

    friend class RandomDecisionTree;

public:
    RandomDecisionForest()
    {
        srand(time(NULL));
    }

    ~RandomDecisionForest()
    {
        for(auto *DT : m_forest)
            delete DT;
        m_forest.clear();
    }

    void readTrainingImageFiles();
    void readTestImageFiles();
    void printPixelCloud();
    void printPixel(pixel_ptr px);
    int pixelCloudSize();
    cv::Mat getPixelImage(pixel_ptr px);
    void imageToPixels(std::vector<pixel_ptr>& res, const cv::Mat &image,imageinfo_ptr img_inf);
    cv::Mat colorCoder(const cv::Mat &labelImage, const cv::Mat &InputImage);
    void trainForest();
    void test();
    cv::Mat classify(int index);
    RDFParams &params() { return m_params; }
    void setParams(const RDFParams &params) { m_params = params; }
    DataSet m_DS;
    std::vector<RandomDecisionTree *> m_forest;
    // Keep all images on memory
    std::vector<cv::Mat> m_imagesContainer;

private:
    void createLetterConfidenceMatrix(const cv::Mat &labelImg, cv::Mat &confidenceMat);
    double m_accuracy;
    std::vector<QString> classify_res;
    RDFParams m_params;
    QString m_dir;
    int m_numOfLetters = 0;

signals:
    void classifiedImageAs(int image_no, char label);
    void treeConstructed();
    void resultPercentage(double accuracy);

};

#endif
