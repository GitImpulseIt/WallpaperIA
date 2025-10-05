#ifndef DATE_HELPER_H
#define DATE_HELPER_H

#include <QString>
#include <QDate>

class DateHelper
{
public:
    static QString getCurrentDateString();
    static QString getPreviousDateString(int daysBack);
};

#endif // DATE_HELPER_H
