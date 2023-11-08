#include <exception>

#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QRegularExpression>

#include "JSONFile.hpp"
#include "Exception.hpp"
#include "Logger.hpp"

using namespace std;

#pragma region CTORs and DTORs
    JSONFile::JSONFile(const string& filePath) : FileName{filePath}
    {
        QFile openedFile(FileName.c_str());

        // Read data from *.json file
        if (openedFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            JsonMainObject = QJsonDocument::fromJson(QByteArray::fromStdString(openedFile.readAll().toStdString())).object();
            openedFile.close();
        }
        else
        {
            throw Exception(string("Bad file name; unable to open file \"" + FileName + "\""));
        }
    }
#pragma endregion

#pragma region Getters
    /// @brief Returns string value from key in main .json file structure.
    string JSONFile::GetValue(const string& strThisLayoutParameter) const
    {
        return JsonMainObject.value(QString::fromStdString(strThisLayoutParameter)).toString().toStdString();
    }

    /// @brief Returns Qt JSON object from key in main .json file structure.
    QJsonValue JSONFile::GetObject(const string& strThisLayoutParameter) const
    {
        return JsonMainObject.value(QString::fromStdString(strThisLayoutParameter));
    }

    /// @brief Returns Qt JSON value object by path. Path must begins with `$.`, example `$.MainObject.ChildArray[index].FieldName`.
    QJsonValue JSONFile::Query(const string& strQuery) const
    {
        // Find dollar sign in place of the first character
        if (strQuery.at(0) != '$') throw Exception(string("JSON path doesn't begin with \'$\'"));

        QString     qstrQuery = QString::fromStdString(strQuery);
        QStringList splitList = qstrQuery.split('.');
        splitList.removeFirst();

        QJsonObject currObj   = JsonMainObject;
        QJsonValue  currVal;

        for (int iter = 0; iter < splitList.length(); iter++)
        {
            QString currSplit  = splitList.at(iter);
            LOGSTM << "currSplit : [" << currSplit.toStdString() << ']' << endl;

            // Current value actually is array
            if (currSplit.contains('[') && currSplit.contains(']'))
            {
                LOGSTM << "This is array : [" << currSplit.toStdString() << ']' << endl;
                QRegularExpression regexp("\\[\\d+\\]");

                // Find [xxxx] number of index in array and clear string from [] bracket
                int arrayIndex = regexp.match(currSplit).captured(0).remove('[').remove(']').toInt();
                
                // Try to find QJsonValue in array
                currVal = currObj.value(
                                        currSplit.remove(currSplit.indexOf('['), 
                                                         currSplit.length() - currSplit.indexOf('[')))
                                                         .toArray()
                                                         .at(arrayIndex);
            }
            // Current value may be an object
            else
            {
                LOGSTM << "This is object : [" << currSplit.toStdString() << ']' << endl;
                currVal = currObj.value(currSplit);
            }

            // If not last - then object/array
            if (iter != splitList.length() - 1)
            {
                currObj = currVal.toObject();
            }
            else
            {
                LOGSTM << "This is the end of query : [" << currSplit.toStdString() << ']' << endl;
            }
        }

        return currVal;
    }
#pragma endregion
