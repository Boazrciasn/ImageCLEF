#include "include/Reader.h"
#include <QDir>
#include<QDebug>
#include "QApplication"
#include<QDirIterator>

void Reader::readFromTo(string filename, vector<QString> &imgName)
{
    struct dirent *ent;

    // check for valid directory
    if ((m_dir = opendir(filename.c_str())) != NULL) {

        QString prev = "abdd";

        // read from folder until it is empty
        while((ent = readdir(m_dir)) != NULL)
        {
            // get filename and convert it to string
            string str = (string)ent->d_name;
            size_t pos = str.find(".");
            if (pos == string::npos || pos == 0)
                continue;

//            cout<<str.substr(0,pos)<<endl;
            QString file = QString::fromStdString(str.substr(0,pos));

            if(prev != file){
                imgName.push_back(file);
                prev = file;
            }
        }
        closedir(m_dir);
    }
    else
        cerr<<"Couldn't open directory!"<<endl;
}

void Reader::findImages(QString baseDir, QString query, vector<QString> &foundImages)
{

    query = "*"+query+"*";

    QDirIterator it(baseDir,QStringList()<<query, QDir::Dirs | QDir::NoDotAndDotDot ) ;
    while (it.hasNext()){
        it.next();
        foundImages.push_back(it.filePath());
        qDebug() << it.fileName();
    }






}
