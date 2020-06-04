// Copyright 2014 Vladimir Alyamkin. All Rights Reserved.

#include "VaQuoleAppThread.h"
#include "../Include/VaQuoleUILib.h"
#include "VaQuoleWebPage.h"

#include <QApplication>
#include <QNetworkProxyFactory>
#include <QWebSettings>

#include <QtDebug>
#include <QFile>
#include <QThread>
#include <QWebFrame>

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
	QFile file(QDate::currentDate().toString("VaQuoleUI_dd_MM_yyyy.log"));
	if(!file.open(QIODevice::Append | QIODevice::Text))
	{
		return;
	}

	QTextStream out(&file);
	out << QTime::currentTime().toString("hh:mm:ss.zzz ");

	switch (type)
	{
	case QtDebugMsg:	out << "DBG"; break;
	case QtWarningMsg:  out << "WRN"; break;
	case QtCriticalMsg: out << "CRT"; break;
	case QtFatalMsg:    out << "FTL"; break;
	}

	out << " VaQuoleUI " << msg << '\n';
	out.flush();
}

namespace VaQuole
{

/** Main Qt class object */
static QApplication* pApp = NULL;

VaQuoleUIManager::~VaQuoleUIManager()
{
	qDebug() << "Trying to stop the UI thread..";

	try
	{
		stop();
		qDebug() << "Stopped.";
	}
	catch(...)
	{
		qDebug() << "Exception catched on app thread destruction";
	}
}

void VaQuoleUIManager::run()
{
	// Create
	if (QApplication::instance() == nullptr)
	{
		int argc = 1;
		//char* argv[] = { "VaQuoleApp" };
		char* argv[2];
		argv[0] = new char[100];
		strcpy_s(argv[0], 100, "VaQuoleApp");
		argv[1] = nullptr;

#ifdef VA_DEBUG
		qInstallMessageHandler(myMessageOutput);
#endif

		pApp = new QApplication(argc, argv);
		pApp->setQuitOnLastWindowClosed(false);
		pApp->processEvents();

		// Set network config
		QNetworkProxyFactory::setUseSystemConfiguration (true);
		QWebSettings::globalSettings()->setAttribute(QWebSettings::PluginsEnabled, true);
		QWebSettings::globalSettings()->setAttribute(QWebSettings::AutoLoadImages, true);

		QWebSettings::globalSettings()->setAttribute(QWebSettings::OfflineStorageDatabaseEnabled, true);
		QWebSettings::globalSettings()->setAttribute(QWebSettings::OfflineWebApplicationCacheEnabled, true);
		QWebSettings::globalSettings()->setAttribute(QWebSettings::LocalStorageEnabled, true);

		QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
		QWebSettings::globalSettings()->setAttribute(QWebSettings::ScrollAnimatorEnabled, true);
	}

	while (!m_stop)
	{
		mutex.lock();
		int PagesNum = WebPages.size();
		mutex.unlock();

		for(int i = 0; i < PagesNum; i++)
		{
			// [START] Lock pages list
			mutex.lock();

			VaQuoleWebUI* Page = WebPages.at(i);

			// [START] Lock data to read values
			Page->mutex.lock();

			UIDataKeeper* ExtComm = Page->GetData();
			Q_CHECK_PTR(ExtComm);

			// Create webview if necessary
			VaQuoleWebView* WebView = WebViews.value(ExtComm->ObjectId, NULL);
			if(WebView == NULL)
			{
				WebView = new VaQuoleWebView();
				WebView->setContextMenuPolicy(Qt::NoContextMenu);

				// Constuct page that ignores modal JS dialogs
				VaQuoleWebPage *WebPage = new VaQuoleWebPage(WebView);
				WebView->setPage(WebPage);

				WebView->show();
				WebViews.insert(ExtComm->ObjectId, WebView);
			}

			// Cache data from struct
			QString NewURL = ExtComm->NewURL;

			bool bNewTransparency = ExtComm->bDesiredTransparency;
			int NewWidth = ExtComm->DesiredWidth;
			int NewHeight = ExtComm->DesiredHeight;

			// Check primary visuals update is necessary
			bool bTransparencyChanged = bNewTransparency != WebView->getTransparency();
			bool bSizeChanged = (WebView->width() != NewWidth || WebView->height() != NewHeight);

			// Extract mouse events
			QList<MouseEvent> MouseEvents = ExtComm->MouseEvents;
			QList<KeyEvent> KeyEvents = ExtComm->KeyEvents;

			// Process JavaScript commands
			QPair<QString, QString> ScriptCommand;
			foreach (ScriptCommand, ExtComm->ScriptCommands)
			{
				QVariant ScriptResult = WebView->page()->mainFrame()->evaluateJavaScript(ScriptCommand.second);

				QString ScriptResultStr = ScriptResult.toString();
				if(!ScriptResultStr.isEmpty() && !ScriptResultStr.isNull())
				{
					QPair<QString, QString> ScriptResultPair;
					ScriptResultPair.first = ScriptCommand.first;
					ScriptResultPair.second = ScriptResultStr;

					ExtComm->ScriptResults.append(ScriptResultPair);
				}
			}

			// Extract JavaScript events
			QList< QPair<QString, QString> > ScriptEvents;
			WebView->getCachedEvents(ScriptEvents, true);
			ExtComm->ScriptEvents.append(ScriptEvents);

			// External data update (mark we've read it)
			ExtComm->NewURL = "";
			ExtComm->bTransparent = WebView->getTransparency();
			ExtComm->bPageLoaded = WebView->isLoadFinished();
			ExtComm->Width = WebView->width();
			ExtComm->Height = WebView->height();
			ExtComm->MouseEvents.clear();
			ExtComm->KeyEvents.clear();
			ExtComm->ScriptCommands.clear();

			// Update grabbed view
			UpdateImageBuffer(ExtComm, WebView);

			// [END] Unlock page data
			Page->mutex.unlock();

			// [END] Unlock pages list
			mutex.unlock();

			// Check primary visual changes
			if(bTransparencyChanged || bSizeChanged)
			{
				if( bTransparencyChanged )
				{
					WebView->setTransparent(bNewTransparency);
				}

				if( bSizeChanged )
				{
					WebView->resize(NewWidth, NewHeight);
				}
			}

			// Check URL
			if(!NewURL.isEmpty())
			{
				qDebug() << "Load url:" << NewURL;
				WebView->resetPageLoadState();
				WebView->load(QUrl(NewURL));
			}

			// Process mouse events
			MouseEvent MyMouseEvent;
			foreach (MyMouseEvent, MouseEvents)
			{
				if(MyMouseEvent.button == Qt::NoButton)
				{
					if (MyMouseEvent.bScrollUp)
					{
						VaQuole::simulateMouseWheel(WebView, MyMouseEvent.eventPos, MyMouseEvent.modifiers, false);
					}
					else if (MyMouseEvent.bScrollDown)
					{
						VaQuole::simulateMouseWheel(WebView, MyMouseEvent.eventPos, MyMouseEvent.modifiers, true);
					}
					else
					{
						// It's a mouse move event
						VaQuole::simulateMouseMove(WebView, MyMouseEvent.eventPos);
					}
				}
				else if(MyMouseEvent.button == Qt::RightButton && MyMouseEvent.bButtonPressed == true)
				{
					// We're simulation right mouse click as a context menu request
					// because QWebView doesn't process RMB properly
					VaQuole::simulateContextMenu(WebView, MyMouseEvent.eventPos, MyMouseEvent.modifiers);
				}
				else
				{
					VaQuole::simulateMouseClick(WebView, MyMouseEvent.eventPos, MyMouseEvent.button, MyMouseEvent.modifiers, MyMouseEvent.bButtonPressed);
				}
			}

			// Process key events
			KeyEvent MyKeyEvent;
			foreach (MyKeyEvent, KeyEvents)
			{
				VaQuole::simulateKey(WebView, MyKeyEvent.key, MyKeyEvent.modifiers, MyKeyEvent.text, MyKeyEvent.bKeyPressed);
			}

			// Update pages num to be sure that we're in array bounds
			mutex.lock();
			PagesNum = WebPages.size();
			mutex.unlock();
		}

		// Process Qt events
		qApp->processEvents();

		// Clean pages marked for delete
		mutex.lock();
		PagesNum = WebPages.size();
		for(int j = 0; j < WebPages.size(); )
		{
			if(WebPages.at(j)->GetData()->bMarkedForDelete)
			{
				VaQuoleWebView* ViewToDelete = WebViews.value(WebPages.at(j)->GetData()->ObjectId);
				VaQuoleWebUI* PageToDelete = WebPages.at(j);

				WebViews.remove(WebPages.at(j)->GetData()->ObjectId);
				WebPages.removeAt(j);

				delete ViewToDelete;
				delete PageToDelete->GetData();
				delete PageToDelete;
			}
			else
			{
				j++;
			}
		}
		mutex.unlock();

		// Check we've closed some windows
		if( PagesNum != WebPages.size() )
		{
			qDebug() << "Process pages deletion:" << WebPages.size() << PagesNum;
			qApp->processEvents();
		}
	}

	qDebug() << "About to exit";
}

void VaQuoleUIManager::AddPage(VaQuoleWebUI *Page)
{
	std::lock_guard<std::mutex> guard(mutex);

	WebPages.append(Page);
}

void VaQuoleUIManager::UpdateImageBuffer(UIDataKeeper *ExtComm, VaQuoleWebView *WebView)
{
	if(ExtComm->ImageDataSize != WebView->getImageDataSize())
	{
		if(ExtComm->ImageBits)
		{
			delete ExtComm->ImageBits;
		}

		ExtComm->ImageDataSize = WebView->getImageDataSize();
		ExtComm->ImageBits = new uchar[ExtComm->ImageDataSize];
	}

	// Copy image only if page is enabled!
	if (ExtComm->bEnabled)
	{
		memcpy(ExtComm->ImageBits, WebView->getImageData(), ExtComm->ImageDataSize);
	}
}

} // namespace VaQuole
