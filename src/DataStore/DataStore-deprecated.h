#ifndef DATASTORE_H
#define DATASTORE_H

#include <QString>
#include <map>

template <class T>
class DataStore
{
    private:
        static bool instanced;
        static DataStore<T>* instance;
        std::map<QString, T> data;
        DataStore()
        {

        }

    public:
        static DataStore<T>* getInstance()
        {
            if(!DataStore<T>::instanced)
            {
                DataStore<T>::instanced = true;
                DataStore<T>::instance = new DataStore<T>();
            }
                
            return DataStore<T>::instance;
        }

        void clear();
        void setData(QString key, T value);
        QString getData(QString key);

        ~DataStore()
        {
            DataStore<T>::instanced = false;
        }
};

template <class T>
void DataStore<T>::clear()
{
    this->data.clear();
}

template <class T>
void DataStore<T>::setData(QString key, T value)
{
    this->data[key] = value;
}

template <class T>
QString DataStore<T>::getData(QString key)
{
    return this->data[key];
}
#endif