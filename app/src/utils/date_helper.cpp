#include "date_helper.h"

QString DateHelper::getCurrentDateString()
{
    return QDate::currentDate().toString("dd/MM/yyyy");
}

QString DateHelper::getPreviousDateString(int daysBack)
{
    return QDate::currentDate().addDays(-daysBack).toString("dd/MM/yyyy");
}
