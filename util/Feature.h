#ifndef FEATURE_H
#define FEATURE_H

#include "precompiled.h"

class Feature
{
public:
    static int max_w;
    static int max_h;
    Feature();
    static QVector<MatFeature> features;
    static QVector<QString> features_str;
    static void init();
    static QString getVisual(const MatFeature feature);
    static QString getSeperator(const int width);
};

#endif // FEATURE_H
