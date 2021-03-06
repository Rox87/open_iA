/*************************************  open_iA  ************************************ *
* **********   A tool for visual analysis and processing of 3D CT images   ********** *
* *********************************************************************************** *
* Copyright (C) 2016-2020  C. Heinzl, M. Reiter, A. Reh, W. Li, M. Arikan, Ar. &  Al. *
*                          Amirkhanov, J. Weissenböck, B. Fröhler, M. Schiwarth       *
* *********************************************************************************** *
* This program is free software: you can redistribute it and/or modify it under the   *
* terms of the GNU General Public License as published by the Free Software           *
* Foundation, either version 3 of the License, or (at your option) any later version. *
*                                                                                     *
* This program is distributed in the hope that it will be useful, but WITHOUT ANY     *
* WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A     *
* PARTICULAR PURPOSE.  See the GNU General Public License for more details.           *
*                                                                                     *
* You should have received a copy of the GNU General Public License along with this   *
* program.  If not, see http://www.gnu.org/licenses/                                  *
* *********************************************************************************** *
* Contact: FH OÖ Forschungs & Entwicklungs GmbH, Campus Wels, CT-Gruppe,              *
*          Stelzhamerstraße 23, 4600 Wels / Austria, Email: c.heinzl@fh-wels.at       *
* ************************************************************************************/
#pragma once

#include "ui_Mainwindow.h"
#include "open_iA_Core_export.h"

#include "iAPreferences.h"
#include "iARenderSettings.h"
#include "iASlicerSettings.h"
#include "iAVolumeSettings.h"
#include "io/iARawFileParameters.h"

#include <QMainWindow>
#include <QMdiSubWindow>
#include <QSharedPointer>

class QAction;
class QActionGroup;
class QComboBox;
class QDomDocument;
class QDomElement;
class QDomNode;
class QMenu;
class QMdiArea;
class QLabel;
class QSplashScreen;

class vtkCamera;
class vtkImageData;

class iAChartTransferFunction;
class iAModalityList;
class iAModuleDispatcher;
class iAXmlSettings;
class MdiChild;

//! Application main window, provides access to all global graphical user interface elements.
class open_iA_Core_API MainWindow : public QMainWindow, public Ui_MainWindow
{
	Q_OBJECT

public:
	MainWindow(QString const & appName, QString const & version, QString const& buildInformation, QString const & splashImage);
	~MainWindow() override;
	static int runGUI(int argc, char * argv[], QString const & appName, QString const & version, QString const& buildInformation,
		QString const & splashPath, QString const & iconPath);
	static void initResources();

	void setPath(QString const & p);
	QString const & path();
	void setCurrentFile(const QString &fileName);
	QString const & currentFile();  //!< deprecated. Use a specific mdichilds, or even better, an mdichilds dlg_modalities methods instead!

	void loadFile(QString const & fileName);
	void loadFile(QString fileName, bool isStack);
	void loadFiles(QStringList fileNames);

	void saveCamera(iAXmlSettings & xml);
	bool loadCamera(iAXmlSettings & xml);
	void saveSliceViews(iAXmlSettings & xml);
	void saveSliceView(QDomDocument &doc, QDomNode &sliceViewsNode, vtkCamera *ren, QString const & elemStr);
	void loadSliceViews(QDomNode sliceViewsNode);
	void saveTransferFunction(QDomDocument &doc, iAChartTransferFunction* transferFunction);
	void savePreferences(iAXmlSettings &xml);
	void loadPreferences(QDomNode preferencesNode);
	void saveRenderSettings(iAXmlSettings &xml);
	void loadRenderSettings(QDomNode renderSettingsNode);
	void saveSlicerSettings(iAXmlSettings &xml);
	void loadSlicerSettings(QDomNode slicerSettingsNode);
	//! Get the File menu (can be used by modules to append entries to it).
	QMenu * fileMenu();
	//! Get the Filters menu (can be used by modules to append entries to it).
	QMenu * filtersMenu();
	//! Get the Tools menu (can be used by modules to append entries to it).
	QMenu * toolsMenu();
	//! Get the Help menu (can be used by modules to append entries to it).
	QMenu * helpMenu();
	//! @{ Get access to result child with the given title.
	//! (depending on preferences, this will either open a new mdi child window, or reuse the currently active one)
	MdiChild * resultChild( QString const & title );
	MdiChild * resultChild( int childInd, QString const & title );
	MdiChild * resultChild( MdiChild* oldChild, QString const & title );
	//! @}
	//! Provides access to the currently active mdi child, if such is available.
	//! @return pointer to the currently active mdi child, or nullptr if no child is currently open
	MdiChild * activeMdiChild();
	//! Provides access to a second loaded mdi child, if such is available.
	//! Will throw an error if none is available or more than two are loaded.
	//! @deprecated instead of this method, in filters, use the facilities
	//!     provided in iAFilter (via the requiredInputs parameter to the constructor) to specify multiple inputs
	MdiChild * secondNonActiveChild();
	//! Get list of the titles of currently open MdiChild windows.
	QList<QString> mdiWindowTitles();
	//! Get the list of current MdiChild windows.
	QList<MdiChild*> mdiChildList(QMdiArea::WindowOrder order = QMdiArea::CreationOrder);
	//! Get the list of current child windows of type T.
	template <typename T> QList<T*> childList(QMdiArea::WindowOrder order = QMdiArea::CreationOrder);
	//! Get the active child window of type T.
	template <typename T> T * activeChild();
	QMdiSubWindow* addSubWindow(QWidget * child);
	void loadArguments(int argc, char** argv);
	iAPreferences const & getDefaultPreferences() const;
	iAModuleDispatcher& getModuleDispatcher() const;
	MdiChild *createMdiChild(bool unsavedChanges);
	void closeMdiChild(MdiChild* child);
	void closeAllSubWindows();
	void updateInteractionModeControls(int mode);

protected:
	void closeEvent(QCloseEvent *event) override;
	void dragEnterEvent(QDragEnterEvent *e) override;
	void dropEvent(QDropEvent *e) override;

private slots:
	void quitTimerSlot();
	void hideSplashSlot();
	void open();
	void openRaw();
	void openImageStack();
	void openVolumeStack();
	void openWithDataTypeConversion();
	void openTLGICTData();
	void save();
	void saveAs();
	void loadSettings();
	bool saveSettings();
	void saveProject();
	void maxXY();
	void maxXZ();
	void maxYZ();
	void maxRC();
	void multi();
	void prefs();
	void linkViews();
	void linkMDIs();
	void enableInteraction();
	void toggleConsole();
	void toggleFullScreen();
	void toggleMenu();
	void renderSettings();
	void slicerSettings();
	void loadTransferFunction();
	void saveTransferFunctionSlot();
	void deletePoint();
	void changeColor();
	void resetView();
	void resetTrf();
	void changeInteractionMode(bool isChecked);
	void meshDataMovable(bool isChecked);
	void toggleSnakeSlicer(bool isChecked);
	void toggleMagicLens(bool isChecked);
	void toggleMagicLens3D(bool isChecked);
	void rendererCamPosition();
	void raycasterAssignIso();
	void raycasterSaveCameraSettings();
	void raycasterLoadCameraSettings();
	void openRecentFile();
	void childClosed();
	void toggleMainWindowStatusBar();
	void toggleChildStatusBar();
	void toggleToolbar();
	void about();
	void buildInformation();
	void wiki();
	void saveLayout();
	void resetLayout();
	void deleteLayout();
	void toggleSliceProfile(bool isChecked);
	void updateMenus();
	void updateWindowMenu();
	void setActiveSubWindow(QWidget *window);
	void pointSelected();
	void noPointSelected();
	void endPointSelected();
	void setHistogramFocus();
	void consoleVisibilityChanged(bool newVisibility);

public slots:
	void loadLayout();

signals:
	void styleChanged();
	void fullScreenToggled();

private:
	void connectSignalsToSlots();
	void readSettings();
	void writeSettings();
	void createRecentFileActions();
	void updateRecentFileActions();
	void applyQSS();
	void setModuleActionsEnabled( bool isEnabled );
	void loadCamera(QDomNode const & node, vtkCamera* camera);
	void saveCamera(QDomElement &cameraElement, vtkCamera* camera);
	void copyFunctions(MdiChild* oldChild, MdiChild* newChild);
	void loadTLGICTData(QString const & baseDirectory);
	bool keepOpen();
	MdiChild* findMdiChild(const QString &fileName);

	static const int MaxRecentFiles = 8;

	QSplashScreen *m_splashScreen;
	QAction *m_separatorAct;
	QAction *m_recentFileActs[MaxRecentFiles];
	QActionGroup *m_slicerToolsGroup;
	QString m_qssName;
	iAVolumeSettings m_defaultVolumeSettings;
	iARenderSettings m_defaultRenderSettings;
	iASlicerSettings m_defaultSlicerSettings;
	iAPreferences m_defaultPreferences;

	//! @{ Open with DataType Conversion settings
	unsigned int m_owdtcs;
	int m_owdtcxori, m_owdtcyori, m_owdtczori,
		m_owdtcxsize, m_owdtcysize, m_owdtczsize,
		m_owdtcdov;
	iARawFileParameters m_rawFileParams;
	double m_owdtcoutmin, m_owdtcoutmax, m_owdtcmin, m_owdtcmax;
	//! @}

	bool m_lpCamera, m_lpSliceViews, m_lpTransferFunction, m_lpProbabilityFunctions, m_lpPreferences, m_lpRenderSettings, m_lpSlicerSettings;
	bool m_spCamera, m_spSliceViews, m_spTransferFunction, m_spProbabilityFunctions, m_spPreferences, m_spRenderSettings, m_spSlicerSettings;

	QString m_defaultLayout;
	QString m_curFile, m_path;
	QTimer *m_splashTimer, *m_quitTimer;
	QComboBox * m_layout;
	QScopedPointer<iAModuleDispatcher> m_moduleDispatcher;
	QStringList m_layoutNames;
	QString m_gitVersion, m_buildInformation;
};

template <typename T> QList<T*> MainWindow::childList(QMdiArea::WindowOrder order)
{
	QList<T*> res;
	for (QMdiSubWindow *window: mdiArea->subWindowList(order))
	{
		T * child = dynamic_cast<T*>(window->widget());
		if (child)
		{
			res.append(child);
		}
	}
	return res;
}

template <typename T> T * MainWindow::activeChild()
{
	int subWndCnt = childList<T>().size();
	if (subWndCnt > 0)
	{
		return childList<T>(QMdiArea::ActivationHistoryOrder).last();
	}
	return nullptr;
}
