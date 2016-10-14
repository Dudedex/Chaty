#include "chat_style_helper.h"
#include <QDebug>

ChatStyleHelper::ChatStyleHelper()
{
}

QString ChatStyleHelper::getFormattedUserMessage(std::string timestamp, std::string message)
{
    QString html = QString("<table width=80% border=1 bgcolor=\"#00ffaa\" align=left>"
                           "<tr>"
                           "<td>"
                           "<table width=80% >"
                           "<tr>"
                           "<th width=70% align=left bgcolor=\"#00ffaa\">"+ CryptoHelper::saveStringConversion(timestamp) + "</th>"
                           "<th width=30% align=right bgcolor=\"#00ffaa\">YOU</th>"
                           "</tr>"
                           "<tr>"
                           "<td colspan=2 width=100% bgcolor=\"#00ffaa\">" + CryptoHelper::saveStringConversion(message) + "</td>"
                           "</tr></table></td></tr></table><br>");
    return html;
}

QString ChatStyleHelper::getFormattedReceivedMessage(std::string transmitter, std::string timestamp, std::string message)
{
    QString html = QString("<table width=80% border=1 bgcolor=\"#FF8953\" align=right>"
                           "<tr>"
                           "<td>"
                           "<table width=80% >"
                           "<tr>"
                           "<th width=40% align=left bgcolor=\"#FF8953\">" + CryptoHelper::saveStringConversion(transmitter) + "</th>"
                           "<th width=60% align=right bgcolor=\"#FF8953\">" + CryptoHelper::saveStringConversion(timestamp) + "</th>"
                           "</tr>"
                           "<tr>"
                           "<td colspan=2 width=100% bgcolor=\"#FF8953\">" + CryptoHelper::saveStringConversion(message) + "</td>"
                           "</tr></table></td></tr></table><br>");
    return html;
}
