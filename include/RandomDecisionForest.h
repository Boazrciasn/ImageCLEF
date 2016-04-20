#ifndef CPV_RANDOM_DECISION_FOREST
#define CPV_RANDOM_DECISION_FOREST


#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "QString"
#include <QDir>
#include <QDebug>
#include "QApplication"
#include <QDirIterator>
#include <include/Util.h>

#include <dirent.h>
#include <iostream>

#include <time.h>       /* time */

#define MIN_ENTROPY 0.05
#define MAX_DEPTH 16

struct Coord{
    int m_r,m_c;
    Coord(){m_r=0;m_c=0;}
    Coord(int x, int y) :
        m_r(x), m_c(y) {}
    Coord(const Coord& crd) : m_r(crd.m_r), m_c(crd.m_r) {}
};

struct ImageInfo
{
    char label;
    int sampleId;
    ImageInfo(char lbl, int smplId) :
        label(lbl), sampleId(smplId){}
};

struct Pixel
{
    Coord position;
    unsigned char intensity;
    ImageInfo* imgInfo;
    Pixel(Coord crd, unsigned char intnsty, ImageInfo* imgInf):
        position(crd), intensity(intnsty),imgInfo(imgInf){}
};

struct Node
{
    int tau;
    Coord teta1, teta2;
    bool isLeaf;
    int id;
    cv::Mat hist;
};

using Tree = std::vector<Node>;

class RandomDecisionTree
{

public:
    RandomDecisionTree() :  m_tree(pow(2, MAX_DEPTH)-1){}
    int m_depth;
    int m_numberofleaves;
    Tree m_tree;
};

class RandomDecisionForest{

public:

    RandomDecisionForest(int probe_dstanceX,int probe_dstanceY) : m_tempTree(pow(2, MAX_DEPTH)-1)
    {
        m_probe_distanceX = probe_dstanceX;
        m_probe_distanceY = probe_dstanceY;
        srand (time(NULL));
        min_InfoGain = 1;
        max_InfoGain = -1;

    }

    void readTrainingImageFiles();
    void readTestImageFiles();
    void printPixelCloud();
    void printPixel(Pixel* px);
    void generateTeta(Coord& crd);
    int generateTau();

    inline float calculateEntropy(cv::Mat& hist)
    {
        float entr = 0;
        float totalSize=0;
        int nCols = hist.cols;

        for(int i=0; i<nCols; ++i)
            totalSize += hist.at<float>(0, i);

        for(int i=0; i<hist.cols; ++i)
        {
            float nPixelsAt = hist.at<float>(0, i);
            if(nPixelsAt>0)
            {
                float probability = nPixelsAt/totalSize;
                entr += probability*(log2(1.0f/probability));
            }
        }
        //cout << "ENTROPY : " << entr;
        return entr;
    }

    inline float calculateEntropyOfVector(std::vector<Pixel*>& pixels)
    {
        cv::Mat hist = createHistogram(pixels);
        return calculateEntropy(hist);
    }

    // creates histogram out of a pixel vector : need(?) fix after image info re-arrange.
    inline cv::Mat createHistogram(std::vector<Pixel*>& pixels){
        cv::Mat hist = cv::Mat::zeros(1,m_labelCount,cv::DataType<float>::type);
        for (std::vector<Pixel*>::iterator it = pixels.begin() ; it != pixels.end(); ++it)
        {
            Pixel* px = *it;
            int index = letterIndex(px->imgInfo->label);
            hist.at<float>(0,index)++;
        }
        return hist;
    }

    inline bool isLeft(Pixel* p, Node& node, cv::Mat& img)
    {
        int new_teta1R = node.teta1.m_r + p->position.m_r;
        int new_teta1C = node.teta1.m_c + p->position.m_c;
        int intensity1 = img.at<uchar>(new_teta1R,new_teta1C);

        int new_teta2R = node.teta2.m_r + p->position.m_r ;
        int new_teta2C = node.teta2.m_c + p->position.m_c ;
        int intensity2 = img.at<uchar>(new_teta2R,new_teta2C);

        return ((intensity1- intensity2) <= node.tau);
    }

    inline void divide(std::vector<Pixel*>& parentPixels, std::vector<Pixel*>& left, std::vector<Pixel*>& right, Node& parent)
    {
        for (auto px : parentPixels)
        {
            auto img = m_trainImagesVector[px->imgInfo->sampleId];
            (isLeft(px, parent, img) ? left : right).push_back(px);
        }
    }

    void trainTree();
    bool pixelSizesConsistent();

    inline int getTotalNumberOfPixels(const cv::Mat& hist)
    {
        int totalSize =0;
        int nCols = hist.cols;
        for(int i=0; i<nCols; ++i)
            totalSize += hist.at<float>(0, i);
        return totalSize;
    }


    inline cv::Mat unpad(cv::Mat img)
    {
        int width = img.cols-2*m_probe_distanceX, height = img.rows-2*m_probe_distanceY;
        cv::Rect roi_rect = cv::Rect(m_probe_distanceX, m_probe_distanceY, width, height );
        cv::Mat unpadded ;
        img(roi_rect).copyTo(unpadded);
        return unpadded;

    }

    cv::Mat classify(int index);
    void test();
    bool test(const cv::Mat& image, char letter, const Tree &tree);

    void constructTree(Node& root, std::vector<Pixel*>& pixels);
    void tuneParameters(std::vector<Pixel*>& parentPixels, Node& parent);
    int pixelCloudSize();
    void printHistogram(cv::Mat& hist);
    void printNode(Node& node);
    void printTree(RandomDecisionTree tree);
    cv::Mat getPixelImage(Pixel* px);
    inline Node getLeafNode(Pixel*px, int nodeId, const Tree &tree)
    {
        Node root = tree[nodeId];
        if(root.isLeaf)
        {
            // qDebug()<<"LEAF REACHED :"<<root.id;
            return root;
        }
        cv::Mat img = m_testImagesVector[px->imgInfo->sampleId];
        int childId = root.id *2 ;
        //qDebug()<<"LEAF SEARCH :"<<root.id << " is leaf : " << root.isLeaf;
        if(!isLeft(px,root,img))
            childId++;
        return getLeafNode(px,childId-1, tree);
    }

    inline int getMaxLikelihoodIndex(const cv::Mat& hist)
    {
        int max_val=-1;
        int max_index=0;
        for(int i=0;i<hist.cols;++i)
        {
            if(hist.at<float>(0, i) > max_val)
            {
                max_val = hist.at<float>(0, i);
                max_index = i;
            }
        }
        return max_index;
    }

    inline int letterIndex(char letter)
    {
        return letter-'a';
    }

    inline void getImageLabelVotes(const cv::Mat& label_image, cv::Mat& vote_vector)
    {
        if(vote_vector.cols != m_labelCount)
        {
            std::cerr<<"Assertion Failed BRO!  Number of labels mismatch"<< std::endl;
            return;
        }
        int rows = label_image.rows;
        int cols = label_image.cols;
        for (int i = 0; i < rows ; ++i)
            for (int j = 0; j < cols; ++j)
                ++vote_vector.at<float>(label_image.at<uchar>(i,j));
    }


    void imageToPixels(std::vector<Pixel*>& res, const cv::Mat &image,ImageInfo* img_inf);
    cv::Mat colorCoder(const cv::Mat &labelImage, const cv::Mat &InputImage);
    void setTrainPath(QString path);
    void setTestPath(QString path);
    void trainForest();

    void setNumberofTrees(int no_of_trees){
        m_no_of_trees = no_of_trees;
    }

    void setNumberofLabels(int no_of_labels){
        m_labelCount = no_of_labels;
    }

    void setMaxDepth(int max_depth){
        m_maxDepth = max_depth;
    }

    void setMaxIterationForDivision(int max_iteration){
        m_maxIterationForDivision = max_iteration;
    }

    void setMinimumLeafPixelCount(unsigned int min_leaf_pixel_count){
        m_minLeafPixelCount = min_leaf_pixel_count;
    }

    void setPixelsPerImage(int pixel_per_image_count){
        m_pixelsPerImage = pixel_per_image_count;
    }


    std::vector<Pixel*> pixelCloud;
    std::vector<cv::Mat> m_trainImagesVector;
    std::vector<cv::Mat> m_testImagesVector;
    std::vector<RandomDecisionTree> m_forest;
    int m_no_of_trees;
    Tree m_tempTree;
    std::vector<Pixel*> m_pixelCloud;
    // Keep all images on memory
    std::vector<cv::Mat> m_imagesContainer;
    // Test and Train dirs :
    QString m_testpath, m_trainpath;
private:

    void subSample();

    QString m_dir;
    std::vector<char> m_labels;
    int m_probe_distanceX, m_probe_distanceY;
    int m_numOfLetters = 0;
    int m_numOfLeaves ;
    int m_depth;
    float min_InfoGain;
    float max_InfoGain;
    int m_maxDepth;
    int m_pixelsPerImage;
    unsigned int m_minLeafPixelCount;
    int m_labelCount;
    int m_maxIterationForDivision;

};

#endif
