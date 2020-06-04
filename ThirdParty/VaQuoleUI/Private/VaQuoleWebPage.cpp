// Copyright 2014 Vladimir Alyamkin. All Rights Reserved.

#include "VaQuoleWebPage.h"

#include <QDebug>

namespace VaQuole
{

VaQuoleWebPage::VaQuoleWebPage(QObject *parent) :
	QWebPage(parent)
{

}

QString	VaQuoleWebPage::chooseFile(QWebFrame*, const QString & suggestedFile)
{
	qDebug() << "chooseFile:" << suggestedFile;

	return suggestedFile;
}

void VaQuoleWebPage::javaScriptAlert(QWebFrame*, const QString & msg)
{
	qDebug() << "JsAlert:" << msg;
}

bool VaQuoleWebPage::javaScriptConfirm(QWebFrame*, const QString&)
{
	return false;
}

void VaQuoleWebPage::javaScriptConsoleMessage(const QString & message, int lineNumber, const QString & sourceID)
{
	qDebug() << "JsConsole [" << lineNumber << "] [" << sourceID << "]:" << message;
}

bool VaQuoleWebPage::javaScriptPrompt(QWebFrame*, const QString&, const QString&, QString*)
{
	return false;
}

} // namespace VaQuole
