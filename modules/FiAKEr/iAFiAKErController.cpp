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
#include "iAFiAKErController.h"

#include "iAFiberCharData.h"
#include "iAFiberData.h"     // for samplePoints
#include "iAJobListView.h"
#include "iAMeasureSelectionDlg.h"
#include "iARefDistCompute.h"
#include "iAStackedBarChart.h"
#include "ui_DissimilarityMatrix.h"

// FeatureScout:
#include "iA3DCylinderObjectVis.h"
#include "iA3DEllipseObjectVis.h"
#include "iACsvConfig.h"
#include "iACsvVectorTableCreator.h"
#include "iAFeatureScoutModuleInterface.h"
#include "iAVectorPlotData.h"

// Core:
#include <charts/iAChartWidget.h>
#include <charts/iAHistogramData.h>
#include <charts/iAPlotTypes.h>
#include <charts/iAScatterPlot.h> // for selection mode: iAScatterPlot::Rectangle
#include <charts/iAQSplom.h>
#include <charts/iASPLOMData.h>
#include <iAColorTheme.h>
#include <iAConnector.h>
#include <iAConsole.h>
#include <iALookupTable.h>
#include <iALUT.h>
#include <iAMapperImpl.h>
#include <iAMathUtility.h>
#include <iAModuleDispatcher.h>
#include <iARenderer.h>
#include <iARendererManager.h>
#include <iAStringHelper.h>
#include <iAToolsVTK.h>    // for setCamPos
#include <iATransferFunction.h>
#include <iAVolumeRenderer.h>
#include <io/iAFileChooserWidget.h>
#include <io/iAIOProvider.h>
#include <io/iAITKIO.h>
#include <mainwindow.h>
#include <mdichild.h>
#include <qthelper/iADockWidgetWrapper.h>
#include <qthelper/iAFixedAspectWidget.h>
#include <qthelper/iAQTtoUIConnector.h>
#include <qthelper/iASignallingWidget.h>
#include <qthelper/iAVtkQtWidget.h>

#include <vtkCamera.h>
#include <vtkColorTransferFunction.h>
#include <vtkCubeSource.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkIdTypeArray.h>
#include <vtkImageData.h>
#include <vtkLine.h>
#include <vtkPiecewiseFunction.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRendererCollection.h>
#include <vtkRenderer.h>
#include <vtkTable.h>
#include <vtkUnsignedCharArray.h>
#include <vtkVersion.h>
#include <vtkVertexGlyphFilter.h>

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QListView>
#include <QMenu>
#include <QMessageBox>
#include <QModelIndex>
#include <QMouseEvent>
#include <QRadioButton>
#include <QScrollArea>
#include <QSettings>
#include <QSlider>
#include <QSpinBox>
#include <QStandardItemModel>
#include <QTextStream>
#include <QTimer>
#include <QTreeView>

#include <QtGlobal> // for QT_VERSION

#include <array>
#include <utility>    // for pair

namespace
{
	const int ControlSpacing = 4;
	const int ResultListMargin = 2;
	const int HistogramMinWidth = 80;
	const int StackedBarMinWidth = 70;
	const int DefaultPlayDelay = 1000;
	int HistogramBins = 20;
	int SelectionOpacity = iA3DLineObjectVis::DefaultSelectionOpacity;
	int ContextOpacity = iA3DLineObjectVis::DefaultContextOpacity;
	const double MinDiameterFactor = 0.02;
	const double MaxDiameterFactor = 2;
	const int MinFactorSliderVal = 1;
	const int MaxFactorSliderVal = 100;
	double DiameterFactor = 1.0;
	double ContextDiameterFactor = 1.0;
	const size_t NoPlotsIdx = std::numeric_limits<size_t>::max();
	const size_t NoResult = std::numeric_limits<size_t>::max();
	const QString RefMarker(" (Reference)");


	const QString DefaultResultColorTheme("Brewer Accent (max. 8)");
	const QString DefaultStackedBarColorTheme("Material red (max. 10)");

	const int DistributionRefAlpha = 80;
	const QColor OptimStepMarkerColor(192, 0, 0);
	const QColor SelectionColor(0, 0, 0);

	int NameActionColumn = 0;
	int PreviewColumn = 1;
	int StackedBarColumn = 2;
	int HistogramColumn = 3;

	// { SETTING NAMES:
	const QString ProjectFileFolder("Folder");
	const QString ProjectFileFormatName("Format");
	const QString ProjectFileReference("Reference");
	const QString ProjectFileStepShift("StepShift");
	const QString ProjectFileSaveFormatName("CsvFormat");
	const QString ProjectUseStepData("UseStepData");
	const QString ProjectShowPreviews("ShowPreviews");
	const QString CameraPosition("CameraPosition");
	const QString CameraViewUp("CameraViewUp");
	const QString CameraFocalPoint("CameraFocalPoint");
	const QString WindowMaximized("WindowMaximized");
	const QString WindowGeometry("WindowGeometry");
	const QString WindowState("WindowState");

	// General:
	const QString ProjectResultColors("ResultColors");
	const QString ProjectDistributionColors("DistributionColors");
	const QString ProjectReferenceVolume("ReferenceVolume");

	// 3D View:
	const QString ProjectDefaultOpacity("DefaultOpacity");
	const QString ProjectContextOpacity("ContextOpacity");
	const QString ProjectDefaultDiameterFactor("DefaultDiameterFactor");
	const QString ProjectContextDiameterFactor("ContextDiameterFactor");
	const QString ProjectShowBoundingBox("ShowBoundingBox");
	const QString ProjectBoundingBoxBorders("BoundingBoxBorders");
	const QString ProjectShowFiberContext("ShowFiberContext");
	const QString ProjectMergeFiberContexts("MergeFiberContexts");
	const QString ProjectContextSpacing("ContextSpacing");
	const QString ProjectSelectionMode("SelectionMode");
	const QString ProjectRefMatchMetric("ReferenceMatchMetric");
	const QString ProjectShowMatchingReferenceFibers("ShowMatchingReferenceFibers");
	const QString ProjectNumberOfMatchingReferenceFibers("NumberOfMatchingReferenceFibers");
	const QString ProjectConnectMatchingReferenceFibers("ConnectMatchingReferenceFibers");
	// Result List:
	const QString ProjectShowRefInDistribution("ShowReferenceInDistribution");
	const QString ProjectLinkPreviews("LinkPreviews");
	const QString ProjectDistributionHistogramBins("DistributionHistogramBins");
	const QString ProjectDistributionPlotTypes("DistributionPlotTypes");
	const QString ProjectStackedBarChartColors("StackedBarChartColors");
	const QString ProjectDoColorBy("DoColorBy");
	const QString ProjectColorBySelection("ColorBySelection");
	const QString ProjectVisibleResults("VisibleResults");
	const QString ProjectVisibleBoundingBoxes("VisibleBoundingBoxes");
	// Step Charts:
	const QString ProjectAnimationDelay("AnimationDelay");
	const QString ProjectVisibleStepCharts("VisibleStepCharts");
	const QString ProjectCurrentStep("CurrentStep");
	// }
}

const QString iAFiAKErController::FIAKERProjectID("FIAKER");

iAFiAKErController::iAFiAKErController(MainWindow* mainWnd, MdiChild* mdiChild) :
	m_renderManager(new iARendererManager()),
	m_resultColorTheme(iAColorThemeManager::instance().theme(DefaultResultColorTheme)),
	m_mainWnd(mainWnd),
	m_mdiChild(mdiChild),
	m_referenceID(NoResult),
	m_colorByThemeName(iALUT::GetColorMapNames()[0]),
	m_showFiberContext(false),
	m_mergeContextBoxes(false),
	m_showWireFrame(false),
	m_showLines(false),
	m_contextSpacing(0.0),
	m_playTimer(new QTimer(mainWnd)),
	m_refDistCompute(nullptr),
	m_cameraInitialized(false),
	m_spm(new iAQSplom())
{
}

void iAFiAKErController::loadProject(QSettings const& projectFile, QString const& fileName)
{
	auto dataFolder = MakeAbsolute(QFileInfo(fileName).absolutePath(), projectFile.value(ProjectFileFolder, "").toString());
	iACsvConfig config;
	auto configName = projectFile.value(ProjectFileFormatName, "").toString();
	if (!configName.isEmpty())
	{  // old format, load from given format name:
		config = getCsvConfig(configName);
	}
	else
	{  // load full format
		if (!config.load(projectFile, ProjectFileSaveFormatName))
		{
			DEBUG_LOG("Could not load CSV format specification from project file!");
			return;
		}
	}
	// if config name entry exists, load that, otherwise load full config...
	auto stepShift = projectFile.value(ProjectFileStepShift, 0).toDouble();
	auto useStepData = projectFile.value(ProjectUseStepData, true).toBool();
	auto showPreviews = projectFile.value(ProjectShowPreviews, true).toBool();
	start(dataFolder, config, stepShift, useStepData, showPreviews);
}

void iAFiAKErController::start(QString const & path, iACsvConfig const & config, double stepShift, bool useStepData, bool showPreviews)
{
	m_config = config;
	m_config.addClassID = false;
	m_useStepData = useStepData;
	m_showPreviews = showPreviews;
	m_jobs = new iAJobListView();
	m_jobs->layout()->setContentsMargins(1, 0, 0, 0);
	m_jobs->layout()->setSpacing(ControlSpacing);
	m_views.resize(DockWidgetCount);
	m_views[JobView] = new iADockWidgetWrapper(m_jobs, "Jobs", "foeJobs");
	m_mdiChild->addDockWidget(Qt::BottomDockWidgetArea, m_views[JobView]);
	connect(m_mdiChild, &MdiChild::renderSettingsChanged, this, &iAFiAKErController::applyRenderSettings);

	m_data = QSharedPointer<iAFiberResultsCollection>(new iAFiberResultsCollection());
	auto resultsLoader = new iAFiberResultsLoader(m_data, path, m_config, stepShift);
	connect(resultsLoader, &iAFiberResultsLoader::success, this, &iAFiAKErController::resultsLoaded);
	connect(resultsLoader, &iAFiberResultsLoader::failed,  this, &iAFiAKErController::resultsLoadFailed);
	m_jobs->addJob("Loading results...", resultsLoader->progress(), resultsLoader);
	resultsLoader->start();
}

void iAFiAKErController::resultsLoadFailed(QString const & path)
{
	QMessageBox::warning(m_mainWnd, "Fiber Analytics",
		QString("Could not load data in folder '%1'. Make sure it is in the right format. "
			"Make sure to check the Debug Console window for further errors; "
			"for checking the format of a specific csv file, "
			"you can use the data loading dialog provided in the FeatureScout tool.").arg(path));
	delete parent(); // deletes QMdiSubWindow which this widget is child of
	return;
}

void iAFiAKErController::resultsLoaded()
{
	m_resultUIs.resize(m_data->result.size());
	m_selection.resize(m_data->result.size());

	setupMain3DView();
	setupSettingsView();
	auto optimStepsView = setupOptimStepView();
	auto resultListView = setupResultListView();
	auto protocolView = setupProtocolView();
	auto selectionView = setupSelectionView();

	m_views.resize(DockWidgetCount);
	m_views[ResultListView] = new iADockWidgetWrapper(resultListView, "FIAKER Result list", "foeResultList");
	m_views[OptimStepChart] = new iADockWidgetWrapper(optimStepsView, "FIAKER Steps", "foeSteps");
	m_views[SPMView]        = new iADockWidgetWrapper(m_spm, "FIAKER Scatterplot Matrix", "foeSPM");
	m_views[ProtocolView]   = new iADockWidgetWrapper(protocolView, "FIAKER Interactions", "foeInteractions");
	m_views[SelectionView]  = new iADockWidgetWrapper(selectionView, "FIAKER Selections", "foeSelections");
	m_views[SettingsView]   = new iADockWidgetWrapper(m_settingsView, "FIAKER Settings", "foeSettings");

	m_mdiChild->splitDockWidget(m_views[JobView], m_views[ResultListView], Qt::Vertical);
	m_mdiChild->splitDockWidget(m_views[ResultListView], m_views[OptimStepChart], Qt::Vertical);
	m_mdiChild->splitDockWidget(m_views[ResultListView], m_views[SPMView], Qt::Horizontal);
	m_mdiChild->splitDockWidget(m_views[ResultListView], m_views[ProtocolView], Qt::Vertical);
	m_mdiChild->splitDockWidget(m_views[ResultListView], m_views[SelectionView], Qt::Vertical);
	m_mdiChild->splitDockWidget(m_views[ResultListView], m_views[SettingsView], Qt::Vertical);

	m_settingsWidgetMap.insert(ProjectResultColors, m_settingsView->cmbboxResultColors);
	m_settingsWidgetMap.insert(ProjectDistributionColors, m_settingsView->cmbboxDistributionColors);
	m_settingsWidgetMap.insert(ProjectDefaultOpacity, m_settingsView->slOpacityDefault);
	m_settingsWidgetMap.insert(ProjectContextOpacity, m_settingsView->slOpacityContext);
	m_settingsWidgetMap.insert(ProjectDefaultDiameterFactor, m_settingsView->slDiameterFactorDefault);
	m_settingsWidgetMap.insert(ProjectContextDiameterFactor, m_settingsView->slDiameterFactorContext);
	m_settingsWidgetMap.insert(ProjectShowBoundingBox, m_settingsView->cbBoundingBox);
	m_settingsWidgetMap.insert(ProjectBoundingBoxBorders, &m_teBoundingBox);
	m_settingsWidgetMap.insert(ProjectShowFiberContext, m_settingsView->cbFiberContextShow);
	m_settingsWidgetMap.insert(ProjectMergeFiberContexts, m_settingsView->cbFiberContextMerge);
	m_settingsWidgetMap.insert(ProjectContextSpacing, m_settingsView->sbFiberContextSpacing);
	m_settingsWidgetMap.insert(ProjectSelectionMode, m_settingsView->cmbboxSelectionMode);
	m_settingsWidgetMap.insert(ProjectRefMatchMetric, m_settingsView->cmbboxSimilarityMeasure);
	m_settingsWidgetMap.insert(ProjectShowMatchingReferenceFibers, m_chkboxShowReference);
	m_settingsWidgetMap.insert(ProjectNumberOfMatchingReferenceFibers, m_spnboxReferenceCount);
	m_settingsWidgetMap.insert(ProjectConnectMatchingReferenceFibers, m_chkboxShowLines);
	m_settingsWidgetMap.insert(ProjectShowRefInDistribution, m_settingsView->cbShowReferenceDistribution);
	m_settingsWidgetMap.insert(ProjectShowPreviews, m_settingsView->cbShowPreviews);
	m_settingsWidgetMap.insert(ProjectLinkPreviews, m_settingsView->cbLinkPreviews);
	m_settingsWidgetMap.insert(ProjectDistributionHistogramBins, m_settingsView->sbHistogramBins);
	m_settingsWidgetMap.insert(ProjectDistributionPlotTypes, m_settingsView->cmbboxDistributionPlotType);
	m_settingsWidgetMap.insert(ProjectStackedBarChartColors, m_settingsView->cmbboxStackedBarChartColors);
	m_settingsWidgetMap.insert(ProjectDoColorBy, m_colorByDistribution);
	m_settingsWidgetMap.insert(ProjectColorBySelection, m_distributionChoice);
	m_settingsWidgetMap.insert(ProjectVisibleResults, &m_showResultVis);
	m_settingsWidgetMap.insert(ProjectVisibleBoundingBoxes, &m_showResultBox);
	m_settingsWidgetMap.insert(ProjectAnimationDelay, m_settingsView->sbAnimationDelay);
	m_settingsWidgetMap.insert(ProjectVisibleStepCharts, &m_chartCB);
	m_settingsWidgetMap.insert(ProjectCurrentStep, m_optimStepSlider);

	applyRenderSettings();
	loadStateAndShow();
}

void iAFiAKErController::setupMain3DView()
{
	m_main3DWidget = m_mdiChild->renderDockWidget()->vtkWidgetRC;
#if VTK_MAJOR_VERSION < 9
	auto renWin = m_main3DWidget->GetRenderWindow();
#else
	auto renWin = m_main3DWidget->renderWindow();
#endif
	m_ren = renWin->GetRenderers()->GetFirstRenderer();
	m_renderManager->addToBundle(m_ren);
	m_style = vtkSmartPointer<iASelectionInteractorStyle>::New();
	m_style->setSelectionProvider(this);
	m_style->assignToRenderWindow(renWin);
	m_style->setRenderer(m_ren);
	connect(m_style.GetPointer(), &iASelectionInteractorStyle::selectionChanged, this, &iAFiAKErController::selection3DChanged);

	m_customBoundingBoxSource = vtkSmartPointer<vtkCubeSource>::New();
	m_customBoundingBoxMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	m_customBoundingBoxActor = vtkSmartPointer<vtkActor>::New();

	m_customBoundingBoxMapper->SetInputConnection(m_customBoundingBoxSource->GetOutputPort());
	m_customBoundingBoxActor->GetProperty()->SetColor(0, 0, 0);
	m_customBoundingBoxActor->GetProperty()->SetRepresentationToWireframe();
	m_customBoundingBoxActor->PickableOff();
	m_customBoundingBoxActor->SetMapper(m_customBoundingBoxMapper);
}

// not ideal - no clear separation between differences between steps and distance metrics!
void iAFiAKErController::addChartCB()
{
	++m_chartCount;
	auto cb = new QCheckBox(diffName(m_chartCount-1));
	cb->setChecked(false);
	cb->setEnabled(false);
	cb->setProperty("chartID", static_cast<qulonglong>(m_chartCount - 1));
	connect(cb, &QCheckBox::stateChanged, this, &iAFiAKErController::optimDataToggled);
	m_settingsView->checkboxContainer->layout()->addWidget(cb);
	m_chartCB.push_back(cb);
	m_optimStepChart.push_back(nullptr);
}

void iAFiAKErController::setupSettingsView()
{
	m_settingsView = new iAFIAKERSettingsWidget();

	m_settingsView->slOpacityDefault->setValue(SelectionOpacity);
	m_settingsView->lbOpacityDefaultValue->setText(QString::number(SelectionOpacity, 'f', 2));

	m_settingsView->slOpacityContext->setValue(ContextOpacity);
	m_settingsView->lbOpacityContextValue->setText(QString::number(ContextOpacity, 'f', 2));

	m_diameterFactorMapper = new iALinearMapper(MinDiameterFactor, MaxDiameterFactor, MinFactorSliderVal, MaxFactorSliderVal);
	m_settingsView->slDiameterFactorDefault->setMinimum(MinFactorSliderVal);
	m_settingsView->slDiameterFactorDefault->setMaximum(MaxFactorSliderVal);
	int factorSliderValue = static_cast<int>(m_diameterFactorMapper->srcToDst(DiameterFactor));
	m_settingsView->slDiameterFactorDefault->setValue(factorSliderValue);
	m_settingsView->lbDiameterFactorDefaultValue->setText(QString::number(m_diameterFactorMapper->dstToSrc(factorSliderValue), 'f', 2));

	m_settingsView->slDiameterFactorContext->setMinimum(MinFactorSliderVal);
	m_settingsView->slDiameterFactorContext->setMaximum(MaxFactorSliderVal);
	int contextFactorSlider = static_cast<int>(m_diameterFactorMapper->srcToDst(ContextDiameterFactor));
	m_settingsView->slDiameterFactorContext->setValue(contextFactorSlider);
	m_settingsView->lbDiameterFactorContextValue->setText(QString::number(m_diameterFactorMapper->dstToSrc(contextFactorSlider), 'f', 2));

	m_teBoundingBox.resize(6);
	m_teBoundingBox[0] = m_settingsView->leBoundingBoxC1X;
	m_teBoundingBox[1] = m_settingsView->leBoundingBoxC1Y;
	m_teBoundingBox[2] = m_settingsView->leBoundingBoxC1Z;
	m_teBoundingBox[3] = m_settingsView->leBoundingBoxC2X;
	m_teBoundingBox[4] = m_settingsView->leBoundingBoxC2Y;
	m_teBoundingBox[5] = m_settingsView->leBoundingBoxC2Z;
	for (int i = 0; i < 6; ++i)
	{
		connect(m_teBoundingBox[i], &QLineEdit::editingFinished, this, &iAFiAKErController::updateBoundingBox);
	}

	m_settingsView->cbShowWireFrame->setChecked(false);

	m_settingsView->sbAnimationDelay->setValue(DefaultPlayDelay);
	m_playTimer->setInterval(DefaultPlayDelay);

	//iAFiberCharData::FiberValueCount               // v Projection error
	m_chartCount = 0;
	addChartCB();
	size_t curPlotStart = 0;
	for (size_t resultID = 0; resultID < m_data->result.size(); ++resultID)
	{
		auto & d = m_data->result[resultID];
		if (!d.projectionError.empty())
		{
			m_resultUIs[resultID].startPlotIdx = curPlotStart;
			curPlotStart += d.fiberCount;
		}
		else
		{
			m_resultUIs[resultID].startPlotIdx = NoPlotsIdx;
		}
	}

	m_showReferenceWidget = new QWidget();
	m_chkboxShowReference = new QCheckBox("Show ");
	m_spnboxReferenceCount = new QSpinBox();
	m_spnboxReferenceCount->setValue(1);
	m_spnboxReferenceCount->setMinimum(1);
	m_spnboxReferenceCount->setMaximum(1);
	m_showReferenceWidget->setLayout(new QHBoxLayout());
	m_showReferenceWidget->layout()->setContentsMargins(0, 0, 0, 0);
	m_showReferenceWidget->layout()->setSpacing(ControlSpacing);
	m_chkboxShowLines = new QCheckBox("Connect");
	connect(m_chkboxShowReference, &QCheckBox::stateChanged, this, &iAFiAKErController::showReferenceToggled);
	connect(m_spnboxReferenceCount, SIGNAL(valueChanged(int)), this, SLOT(showReferenceCountChanged(int)));
	connect(m_chkboxShowLines, &QCheckBox::stateChanged, this, &iAFiAKErController::showReferenceLinesToggled);
	m_showReferenceWidget->layout()->addWidget(m_chkboxShowReference);
	m_showReferenceWidget->layout()->addWidget(m_spnboxReferenceCount);
	m_showReferenceWidget->layout()->addWidget(new QLabel("nearest ref. fibers"));
	m_showReferenceWidget->layout()->addWidget(m_chkboxShowLines);
	m_showReferenceWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	auto grid = static_cast<QGridLayout*>(m_settingsView->tab3DView->layout());
	grid->addWidget(m_showReferenceWidget, grid->rowCount(), 0, 1, 7);

	m_settingsView->sbHistogramBins->setValue(HistogramBins);

	m_settingsView->cmbboxStackedBarChartColors->addItems(iAColorThemeManager::instance().availableThemes());
	m_settingsView->cmbboxStackedBarChartColors->setCurrentText(DefaultStackedBarColorTheme);

	m_settingsView->cmbboxDistributionColors->addItems(iALUT::GetColorMapNames());
	m_settingsView->cmbboxDistributionColors->setCurrentIndex(0);

	m_settingsView->cmbboxResultColors->addItems(iAColorThemeManager::instance().availableThemes());
	m_settingsView->cmbboxResultColors->setCurrentText(DefaultResultColorTheme);

	connect(m_settingsView->slOpacityDefault, &QSlider::valueChanged, this, &iAFiAKErController::mainOpacityChanged);
	connect(m_settingsView->slOpacityContext, &QSlider::valueChanged, this, &iAFiAKErController::contextOpacityChanged);
	connect(m_settingsView->slDiameterFactorDefault, &QSlider::valueChanged, this, &iAFiAKErController::diameterFactorChanged);
	connect(m_settingsView->slDiameterFactorContext, &QSlider::valueChanged, this, &iAFiAKErController::contextDiameterFactorChanged);
	connect(m_settingsView->cbFiberContextShow, &QCheckBox::stateChanged, this, &iAFiAKErController::showFiberContextChanged);
	connect(m_settingsView->cbFiberContextMerge, &QCheckBox::stateChanged, this, &iAFiAKErController::mergeFiberContextBoxesChanged);
	connect(m_settingsView->sbFiberContextSpacing, SIGNAL(valueChanged(double)), this, SLOT(contextSpacingChanged(double)));
	connect(m_settingsView->cbBoundingBox, &QCheckBox::stateChanged, this, &iAFiAKErController::showBoundingBoxChanged);
	connect(m_settingsView->cbShowWireFrame, &QCheckBox::stateChanged, this, &iAFiAKErController::showWireFrameChanged);
	connect(m_settingsView->cbShowLines, &QCheckBox::stateChanged, this, &iAFiAKErController::showLinesChanged);
	connect(m_settingsView->pbSampleSelectedFiber, &QPushButton::pressed, this, &iAFiAKErController::visualizeCylinderSamplePoints);
	connect(m_settingsView->pbHideSamplePoints, &QPushButton::pressed, this, &iAFiAKErController::hideSamplePoints);
	connect(m_settingsView->pbSpatialOverview, &QPushButton::pressed, this, &iAFiAKErController::showSpatialOverviewButton);
	connect(m_settingsView->cmbboxSelectionMode, SIGNAL(currentIndexChanged(int)), this, SLOT(selectionModeChanged(int)));
	connect(m_settingsView->cmbboxSimilarityMeasure, SIGNAL(currentIndexChanged(int)), this, SLOT(showReferenceMeasureChanged(int)));
	connect(m_playTimer, &QTimer::timeout, this, &iAFiAKErController::playTimer);
	connect(m_settingsView->sbAnimationDelay, SIGNAL(valueChanged(int)), this, SLOT(playDelayChanged(int)));
	connect(m_settingsView->sbHistogramBins, SIGNAL(valueChanged(int)), this, SLOT(histogramBinsChanged(int)));
	connect(m_settingsView->cbShowReferenceDistribution, &QCheckBox::stateChanged, this, &iAFiAKErController::showReferenceInChartToggled);
	connect(m_settingsView->cbLinkPreviews, &QCheckBox::stateChanged, this, &iAFiAKErController::linkPreviewsToggled);
	connect(m_settingsView->cmbboxDistributionPlotType, SIGNAL(currentIndexChanged(int)),
		this, SLOT(distributionChartTypeChanged(int)));
	connect(m_settingsView->cmbboxStackedBarChartColors, SIGNAL(currentIndexChanged(QString const &)),
		this, SLOT(stackedBarColorThemeChanged(QString const &)));
	connect(m_settingsView->cmbboxDistributionColors, SIGNAL(currentIndexChanged(QString const &)),
		this, SLOT(distributionColorThemeChanged(QString const &)));
	connect(m_settingsView->cmbboxResultColors, SIGNAL(currentIndexChanged(QString const &)),
		this, SLOT(resultColorThemeChanged(QString const &)));
	connect(m_settingsView->pbSensitivity, &QPushButton::clicked, this, &iAFiAKErController::sensitivitySlot);
}

QWidget* iAFiAKErController::setupOptimStepView()
{
	auto chartContainer = new QWidget();
	m_optimChartLayout = new QVBoxLayout();
	chartContainer->setLayout(m_optimChartLayout);
	chartContainer->setSizeIncrement(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_optimStepSlider = new QSlider(Qt::Horizontal);
	m_optimStepSlider->setMinimum(0);
	m_optimStepSlider->setMaximum(static_cast<int>(m_data->optimStepMax) - 1);
	m_optimStepSlider->setValue(static_cast<int>(m_data->optimStepMax) - 1);
	m_currentOptimStepLabel = new QLabel("");
	m_currentOptimStepLabel->setText(QString::number(static_cast<int>(m_data->optimStepMax) - 1));
	connect(m_optimStepSlider, &QSlider::valueChanged, this, &iAFiAKErController::optimStepSliderChanged);
	QPushButton* playPauseButton = new QPushButton("Play");
	connect(playPauseButton, &QPushButton::pressed, this, &iAFiAKErController::playPauseOptimSteps);

	QWidget* optimStepsCtrls = new QWidget();
	optimStepsCtrls->setLayout(new QHBoxLayout());
	optimStepsCtrls->layout()->addWidget(m_optimStepSlider);
	optimStepsCtrls->layout()->addWidget(m_currentOptimStepLabel);
	optimStepsCtrls->layout()->addWidget(playPauseButton);
	optimStepsCtrls->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	QWidget* optimStepsView = new QWidget();
	optimStepsView->setLayout(new QVBoxLayout());
	optimStepsView->layout()->setContentsMargins(1, 0, 0, 0);
	optimStepsView->layout()->setSpacing(ControlSpacing);
	optimStepsView->layout()->addWidget(chartContainer);
	optimStepsView->layout()->addWidget(optimStepsCtrls);
	return optimStepsView;
}

namespace
{
	QSharedPointer<iA3DColoredPolyObjectVis> create3DVis(vtkRenderer* renderer,
		vtkSmartPointer<vtkTable> table, QSharedPointer<QMap<uint, uint> > mapping, QColor const & color, int objectType,
		std::map<size_t, std::vector<iAVec3f> > const & curvedFiberData)
	{
		switch (objectType)
		{
		case iACsvConfig::Ellipses:  return QSharedPointer<iA3DColoredPolyObjectVis>(new iA3DEllipseObjectVis(renderer, table, mapping, color));
		default:
		case iACsvConfig::Cylinders: return QSharedPointer<iA3DColoredPolyObjectVis>(new iA3DCylinderObjectVis(renderer, table, mapping, color, curvedFiberData));
		}
	}
}

QWidget* iAFiAKErController::setupResultListView()
{
	if (!m_showPreviews)
	{
		StackedBarColumn = 1;
		HistogramColumn = 2;
	}
	int commonPrefixLength = 0, commonSuffixLength = 0;
	QString baseName0;
	for (size_t resultID = 0; resultID < m_data->result.size(); ++resultID)
	{
		QString baseName = QFileInfo(m_data->result[resultID].fileName).completeBaseName();
		if (resultID > 0)
		{
			commonPrefixLength = std::min(commonPrefixLength, greatestCommonPrefixLength(baseName, baseName0));
			commonSuffixLength = std::min(commonSuffixLength, greatestCommonSuffixLength(baseName, baseName0));
		}
		else
		{
			commonPrefixLength = baseName.length();
			commonSuffixLength = baseName.length();
			baseName0 = baseName;
		}
	}
	if (commonPrefixLength == commonSuffixLength)
	{
		commonSuffixLength = 0;
	}
	auto resultListScrollArea = new QScrollArea();
	resultListScrollArea->setWidgetResizable(true);
	auto resultList = new QWidget();
	resultListScrollArea->setWidget(resultList);
	resultListScrollArea->setContentsMargins(0, 0, 0, 0);
	m_resultsListLayout = new QGridLayout();
	m_resultsListLayout->setSpacing(ControlSpacing);
	m_resultsListLayout->setContentsMargins(ResultListMargin, ResultListMargin, ResultListMargin, ResultListMargin);
	m_resultsListLayout->setColumnStretch(StackedBarColumn, static_cast<int>(m_data->result.size()));
	m_resultsListLayout->setColumnStretch(HistogramColumn, static_cast<int>(2 * m_data->result.size()));

	auto colorTheme = iAColorThemeManager::instance().theme(DefaultStackedBarColorTheme);
	m_stackedBarsHeaders = new iAStackedBarChart(colorTheme, true);
	m_stackedBarsHeaders->setMinimumWidth(StackedBarMinWidth);

	QAction* exportDissimilarities = new QAction("Export Dissimilarities", nullptr);
	connect(exportDissimilarities, &QAction::triggered, this, &iAFiAKErController::exportDissimilarities);
	m_stackedBarsHeaders->contextMenu()->addAction(exportDissimilarities);
	auto headerFiberCountAction = new QAction("Fiber Count", nullptr);
	headerFiberCountAction->setProperty("colID", 0);
	headerFiberCountAction->setCheckable(true);
	headerFiberCountAction->setChecked(true);
	connect(headerFiberCountAction, &QAction::triggered, this, &iAFiAKErController::stackedColSelect);
	m_stackedBarsHeaders->contextMenu()->addAction(headerFiberCountAction);
	connect(m_stackedBarsHeaders, &iAStackedBarChart::switchedStackMode, this, &iAFiAKErController::switchStackMode);
	connect(m_stackedBarsHeaders, &iAStackedBarChart::doubleClicked, this, &iAFiAKErController::sortByCurrentWeighting);
	m_stackedBarsHeaders->contextMenu()->addSeparator();

	auto nameActionsLabel = new QLabel("Name/Actions");
	nameActionsLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	m_distributionChoice = new QComboBox();
	QStringList paramNames;
	for (size_t curIdx = 0; curIdx < m_data->m_resultIDColumn; ++curIdx)
	{
		paramNames.push_back(QString("%1 Distribution").arg(m_data->spmData->parameterName(curIdx)));
	}
	m_distributionChoice->addItems(paramNames);
	m_distributionChoice->addItem("Match Quality");
	m_distributionChoice->setCurrentIndex(static_cast<int>((*m_data->result[0].mapping)[iACsvConfig::Length]));
	connect(m_distributionChoice, SIGNAL(currentIndexChanged(int)), this, SLOT(distributionChoiceChanged(int)));
	m_distributionChoice->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	m_colorByDistribution = new QCheckBox("Color by");
	connect(m_colorByDistribution, &QCheckBox::stateChanged, this, &iAFiAKErController::colorByDistrToggled);

	auto histHeader = new QWidget();
	histHeader->setLayout(new QHBoxLayout());
	histHeader->layout()->setContentsMargins(0, 0, 0, 0);
	histHeader->layout()->addWidget(m_colorByDistribution);
	histHeader->layout()->addWidget(m_distributionChoice);

	m_resultsListLayout->addWidget(nameActionsLabel, 0, NameActionColumn);
	if (m_showPreviews)
	{
		m_resultsListLayout->setColumnStretch(PreviewColumn, 1);
		auto previewLabel = new QLabel("Preview");
		previewLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
		m_resultsListLayout->addWidget(previewLabel, 0, PreviewColumn);
	}
	m_resultsListLayout->addWidget(m_stackedBarsHeaders, 0, StackedBarColumn);
	m_resultsListLayout->addWidget(histHeader, 0, HistogramColumn);

	m_showResultVis.resize(m_data->result.size());
	m_showResultBox.resize(m_data->result.size());
	m_resultListSorting.clear();
	for (size_t resultID = 0; resultID < m_data->result.size(); ++resultID)
	{
		auto & d = m_data->result.at(resultID);
		auto & ui = m_resultUIs[resultID];

		QString name = QFileInfo(d.fileName).completeBaseName();
		name = name.mid(commonPrefixLength, name.size() - commonPrefixLength - commonSuffixLength);

		m_showResultVis[resultID] = new QCheckBox(name);
		m_showResultVis[resultID]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
		m_showResultVis[resultID]->setProperty("resultID", static_cast<qulonglong>(resultID));
		m_showResultBox[resultID] = new QCheckBox("Box");
		m_showResultBox[resultID]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
		m_showResultBox[resultID]->setProperty("resultID", static_cast<qulonglong>(resultID));

		ui.nameActions = new iASignallingWidget();
		ui.nameActions->setAutoFillBackground(true);
		ui.nameActions->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		ui.nameActions->setLayout(new QVBoxLayout());
		ui.nameActions->layout()->setContentsMargins(0, 0, 0, 0);
		ui.nameActions->layout()->setSpacing(5);
		ui.topFiller = new QWidget();
		ui.topFiller->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		ui.bottomFiller = new QWidget();
		ui.bottomFiller->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		ui.nameActions->layout()->addWidget(ui.topFiller);
		ui.nameActions->layout()->addWidget(m_showResultVis[resultID]);
		ui.nameActions->layout()->addWidget(m_showResultBox[resultID]);
		ui.nameActions->layout()->addWidget(ui.bottomFiller);

		ui.stackedBars = new iAStackedBarChart(colorTheme);
		ui.stackedBars->setMinimumWidth(StackedBarMinWidth);
		ui.stackedBars->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		connect(m_stackedBarsHeaders, &iAStackedBarChart::weightsChanged, ui.stackedBars, &iAStackedBarChart::setWeights);

		ui.histoChart = new iAChartWidget(resultList, "Fiber Length", "");
		ui.histoChart->setShowXAxisLabel(false);
		ui.histoChart->setMinimumWidth(HistogramMinWidth);
		ui.histoChart->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

		m_resultListSorting.insert(resultID, static_cast<int>(resultID));

		std::map<size_t, std::vector<iAVec3f> > curvedStepInfo;
		if (m_useStepData && d.stepData == iAFiberCharData::CurvedStepData)
		{   // get last step:
			auto & lastStepValues = d.stepValues[d.stepValues.size() - 1];
			//              fibers,      point values (each coordinate is 3 values)
			// convert from std::vector<std::vector<double>> to    (as in lastStepValues)
			//                       fiberid, coordinate
			//              std::map<size_t,  std::vector<iAVec3f>
			for (size_t f = 0; f < d.fiberCount; ++f)
			{
				size_t const numPts = lastStepValues[f].size() / 3;
				std::vector<iAVec3f> fiberCurvePoints(numPts);
				for (size_t p = 0; p < numPts; ++p)
				{
					fiberCurvePoints[p] = iAVec3f(
						static_cast<float>(lastStepValues[f][p * 3]),
						static_cast<float>(lastStepValues[f][p * 3 + 1]),
						static_cast<float>(lastStepValues[f][p * 3 + 2]));
				}
				curvedStepInfo.insert(std::make_pair(f, fiberCurvePoints));
			}
		}

		std::map<size_t, std::vector<iAVec3f> > const & curveInfo =
			(m_useStepData && d.stepData == iAFiberCharData::CurvedStepData) ?
			curvedStepInfo : d.curveInfo;
		QColor resultColor(getResultColor(resultID));

		if (m_showPreviews)
		{
			ui.previewWidget = new iAFixedAspectWidget();
			ui.vtkWidget = ui.previewWidget->vtkWidget();
			auto renWin = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
			renWin->SetAlphaBitPlanes(1);
			auto ren = vtkSmartPointer<vtkRenderer>::New();
			ren->SetBackground(1.0, 1.0, 1.0);
			ren->SetUseDepthPeeling(true);
			ren->SetMaximumNumberOfPeels(10);
			renWin->AddRenderer(ren);
#if VTK_MAJOR_VERSION < 9
			ui.vtkWidget->SetRenderWindow(renWin);
#else
			ui.vtkWidget->setRenderWindow(renWin);
#endif
			ui.vtkWidget->setProperty("resultID", static_cast<qulonglong>(resultID));
			ui.mini3DVis = create3DVis(ren, d.table, d.mapping, resultColor, m_data->objectType, curveInfo);
			ui.mini3DVis->setColor(resultColor);
			ui.mini3DVis->show();
			ren->ResetCamera();
			ui.previewWidget->setProperty("resultID", static_cast<qulonglong>(resultID));
			connect(ui.previewWidget, &iAFixedAspectWidget::dblClicked, this, &iAFiAKErController::referenceToggled);
#if VTK_MAJOR_VERSION < 9
			connect(ui.vtkWidget, &iAVtkWidget::mouseEvent, this, &iAFiAKErController::miniMouseEvent);
#else
#ifndef _MSC_VER
			#warning("VTK >= 9.0 - Fix required for missing mouseEvent signal in QVTKOpenGLNativeWidget")
#else
#pragma message("VTK >= 9.0 - Fix required for missing mouseEvent signal in QVTKOpenGLNativeWidget")
#endif
#endif
			// connect changes to visualizations to an update of the 3D widget:
			// {
			connect(ui.mini3DVis.data(), &iA3DObjectVis::updated, ui.vtkWidget, &iAVtkQtWidget::updateAll);
		}
		ui.main3DVis = create3DVis(m_ren, d.table, d.mapping, resultColor, m_data->objectType, curveInfo);

		const double * b = ui.main3DVis->bounds();
		QString bbox = QString("Bounding box: (x: %1..%2, y: %3..%4, z: %5..%6)")
			.arg(b[0]).arg(b[1]).arg(b[2]).arg(b[3]).arg(b[4]).arg(b[5]);
		ui.nameActions->setToolTip(bbox + "\n"
			"Filename: " + d.fileName + "\n"
			"Visualization details: " + ui.main3DVis->visualizationStatistics());

		ui.stackedBars->setProperty("resultID", static_cast<qulonglong>(resultID));
		ui.histoChart->setProperty("resultID", static_cast<qulonglong>(resultID));
		ui.nameActions->setProperty("resultID", static_cast<qulonglong>(resultID));
		connect(ui.stackedBars, &iAStackedBarChart::dblClicked, this, &iAFiAKErController::referenceToggled);
		connect(ui.histoChart, &iAChartWidget::dblClicked, this, &iAFiAKErController::referenceToggled);
		connect(ui.nameActions, &iASignallingWidget::dblClicked, this, &iAFiAKErController::referenceToggled);
		connect(m_showResultVis[resultID], &QCheckBox::stateChanged, this, &iAFiAKErController::toggleVis);
		connect(m_showResultBox[resultID], &QCheckBox::stateChanged, this, &iAFiAKErController::toggleBoundingBox);


		// iA3DColoredObjectVis::updateRenderer makes sure this connection is only triggered if vis is currently shown:
		connect(ui.main3DVis.data(), &iA3DObjectVis::updated, this, &iAFiAKErController::update3D);
		// }
	}
	updateResultList();

	resultList->setLayout(m_resultsListLayout);
	addStackedBar(0);
	changeDistributionSource((*m_data->result[0].mapping)[iACsvConfig::Length]);

	// to add 1 pixel margin on the left:
	auto outerWidget = new QWidget();
	outerWidget->setLayout(new QHBoxLayout());
	outerWidget->layout()->setContentsMargins(1, 0, 0, 0);
	outerWidget->layout()->setSpacing(0);
	outerWidget->layout()->addWidget(resultListScrollArea);
	return outerWidget;
}

QWidget * iAFiAKErController::setupProtocolView()
{
	m_interactionProtocol = new QTreeView();
	m_interactionProtocol->setHeaderHidden(true);
	m_interactionProtocolModel = new QStandardItemModel();
	m_interactionProtocol->setModel(m_interactionProtocolModel);
	m_interactionProtocol->setEditTriggers(QAbstractItemView::NoEditTriggers);
	QWidget* protocolView = new QWidget();
	protocolView->setLayout(new QHBoxLayout());
	protocolView->layout()->setContentsMargins(1, 0, 0, 0);
	protocolView->layout()->setSpacing(ControlSpacing);
	protocolView->layout()->addWidget(m_interactionProtocol);
	return protocolView;
}

QWidget * iAFiAKErController::setupSelectionView()
{
	m_selectionListModel = new QStandardItemModel();
	m_selectionList = new QListView();
	m_selectionList->setModel(m_selectionListModel);
	connect(m_selectionList, &QListView::clicked, this, &iAFiAKErController::selectionFromListActivated);
	auto selectionListWrapper = new QWidget();
	selectionListWrapper->setLayout(new QVBoxLayout());
	selectionListWrapper->layout()->setContentsMargins(0, 0, 0, 0);
	selectionListWrapper->layout()->setSpacing(ControlSpacing);
	selectionListWrapper->layout()->addWidget(new QLabel("Selections:"));
	selectionListWrapper->layout()->addWidget(m_selectionList);
	m_selectionDetailModel = new QStandardItemModel();
	m_selectionDetailsTree = new QTreeView();
	m_selectionDetailsTree->setHeaderHidden(true);
	m_selectionDetailsTree->setModel(m_selectionDetailModel);
	connect(m_selectionDetailsTree, &QTreeView::clicked, this, &iAFiAKErController::selectionDetailsItemClicked);
	auto selectionDetailWrapper = new QWidget();
	selectionDetailWrapper->setLayout(new QVBoxLayout());
	selectionDetailWrapper->layout()->setContentsMargins(0, 0, 0, 0);
	selectionDetailWrapper->layout()->setSpacing(ControlSpacing);
	selectionDetailWrapper->layout()->addWidget(new QLabel("Details:"));
	selectionDetailWrapper->layout()->addWidget(m_selectionDetailsTree);
	auto selectionView = new QWidget();
	selectionView->setLayout(new QHBoxLayout());
	selectionView->layout()->setContentsMargins(1, 0, 0, 0);
	selectionView->layout()->setSpacing(ControlSpacing);
	selectionView->layout()->addWidget(selectionListWrapper);
	selectionView->layout()->addWidget(selectionDetailWrapper);
	return selectionView;
}

void iAFiAKErController::loadStateAndShow()
{
	addInteraction(QString("Loaded %1 results in folder %2.").arg(m_data->result.size()).arg(m_data->folder));

	// SPM needs an active OpenGL Context (it must be visible when setData is called):
	m_spm->setMinimumWidth(200);
	m_spm->showAllPlots(false);
	//auto np = m_data->spmData->numParams();
	std::vector<char> v(m_data->spmData->numParams(), false);
	v[0] = v[1] = v[2] = true;
	m_spm->setData(m_data->spmData, v);
	m_spm->setSelectionMode(iAScatterPlot::Rectangle);
	m_spm->showDefaultMaxizimedPlot();
	m_spm->setSelectionColor(SelectionColor);
	m_spm->setPointRadius(2.5);
	m_spm->settings.enableColorSettings = true;
	setSPMColorByResult();
	connect(m_spm, &iAQSplom::selectionModified, this, &iAFiAKErController::selectionSPMChanged);
	connect(m_spm, &iAQSplom::lookupTableChanged, this, &iAFiAKErController::spmLookupTableChanged);
	m_views[SettingsView]->hide();
	m_views[ProtocolView]->hide();
	m_views[SelectionView]->hide();
	m_views[JobView]->hide();
	m_showReferenceWidget->hide();

	emit setupFinished();
}

QString iAFiAKErController::stackedBarColName(int index) const
{
	return index == 0 ? "Fiber Count" : diffName(index);
}

void iAFiAKErController::addStackedBar(int index)
{
	QString title = stackedBarColName(index);
	m_stackedBarsHeaders->addBar(title, 1, 1);
	for (size_t resultID=0; resultID<m_resultUIs.size(); ++resultID)
	{
		double value, maxValue;
		if (index == 0)
		{
			value = m_data->result[resultID].fiberCount;
			maxValue = m_data->maxFiberCount;
		}
		else
		{
			value = m_data->result[resultID].avgDifference.size() > 0 ?
			        m_data->result[resultID].avgDifference[index-1] : 0;
			maxValue = m_data->maxAvgDifference[index-1];
		}
		m_resultUIs[resultID].stackedBars->addBar(title, value, maxValue);
	}
	m_resultsListLayout->setColumnStretch(StackedBarColumn, static_cast<int>(m_stackedBarsHeaders->numberOfBars()* m_data->result.size()) );
}

void iAFiAKErController::removeStackedBar(int index)
{
	QString title = stackedBarColName(index);
	m_stackedBarsHeaders->removeBar(title);
	for (size_t resultID = 0; resultID < m_resultUIs.size(); ++resultID)
	{
		m_resultUIs[resultID].stackedBars->removeBar(title);
	}
	m_resultsListLayout->setColumnStretch(StackedBarColumn, static_cast<int>(m_stackedBarsHeaders->numberOfBars()*m_data->result.size()));
}

void iAFiAKErController::updateResultList()
{
	for (size_t resultID = 0; resultID < m_resultUIs.size(); ++resultID)
	{
		auto& ui = m_resultUIs[resultID];
		m_resultsListLayout->removeWidget(ui.nameActions);
		if (ui.previewWidget)
		{
			m_resultsListLayout->removeWidget(ui.previewWidget);
		}
		m_resultsListLayout->removeWidget(ui.stackedBars);
		m_resultsListLayout->removeWidget(ui.histoChart);
		m_resultsListLayout->addWidget(ui.nameActions, m_resultListSorting[resultID] + 1, NameActionColumn);
		if (ui.previewWidget)
		{
			m_resultsListLayout->addWidget(ui.previewWidget, m_resultListSorting[resultID] + 1, PreviewColumn);
		}
		m_resultsListLayout->addWidget(ui.stackedBars, m_resultListSorting[resultID] + 1, StackedBarColumn);
		m_resultsListLayout->addWidget(ui.histoChart, m_resultListSorting[resultID] + 1, HistogramColumn);
	}
}

void iAFiAKErController::setSPMColorByResult()
{
	iALookupTable lut;
	size_t numOfResults = m_data->result.size();
	lut.setRange(0, numOfResults - 1);
	lut.allocate(numOfResults);
	for (size_t i = 0; i < numOfResults; i++)
	{
		lut.setColor(i, getResultColor(i));
	}
	m_spm->setLookupTable(lut, m_data->m_resultIDColumn);
}

void iAFiAKErController::stackedColSelect()
{
	auto source = qobject_cast<QAction*>(QObject::sender());
	size_t colID = source->property("colID").toULongLong();
	QString title = stackedBarColName(colID);
	if (source->isChecked())
	{
		addInteraction(QString("Added %1 to stacked bar chart.").arg(title));
		addStackedBar(colID);
	}
	else
	{
		addInteraction(QString("Removed %1 from stacked bar chart.").arg(title));
		removeStackedBar(colID);
	}
}

void iAFiAKErController::switchStackMode(bool stack)
{
	for (size_t resultID = 0; resultID < m_resultUIs.size(); ++resultID)
	{
		m_resultUIs[resultID].stackedBars->setDoStack(stack);
	}
}

void iAFiAKErController::distributionChoiceChanged(int index)
{
	addInteraction(QString("Changed histogram distribution source to %1.").arg(m_data->spmData->parameterName(index)));
	changeDistributionSource(index);
}

void iAFiAKErController::histogramBinsChanged(int value)
{
	addInteraction(QString("Changed number of histogram bins to %1.").arg(value));
	HistogramBins = value;
	changeDistributionSource(m_distributionChoice->currentIndex());
}

void iAFiAKErController::distributionColorThemeChanged(QString const & colorThemeName)
{
	addInteraction(QString("Changed distribution color theme to '%1'.").arg(colorThemeName));
	m_colorByThemeName = colorThemeName;
	changeDistributionSource(m_distributionChoice->currentIndex());
	m_spm->setColorTheme(colorThemeName);
}

bool iAFiAKErController::matchQualityVisActive() const
{
	size_t colorLookupParam = m_distributionChoice->currentIndex();
	return (colorLookupParam >= m_data->spmData->numParams() - 1);
}

void iAFiAKErController::resultColorThemeChanged(QString const & colorThemeName)
{
	addInteraction(QString("Changed result color theme to '%1'.").arg(colorThemeName));
	m_resultColorTheme = iAColorThemeManager::instance().theme(colorThemeName);

	if (m_showPreviews)
	{
		for (size_t resultID = 0; resultID < m_data->result.size(); ++resultID)
		{
			m_resultUIs[resultID].mini3DVis->setColor(getResultColor(resultID));
		}
	}

	// recolor the optimization step plots:
	for (size_t chartID = 0; chartID < m_optimStepChart.size(); ++chartID)
	{
		if (!m_optimStepChart[chartID])
		{
			continue;
		}
		for (size_t resultID = 0; resultID < m_data->result.size(); ++resultID)
		{
			if (m_resultUIs[resultID].startPlotIdx == NoPlotsIdx)
			{
				continue;
			}
			for (size_t p = 0; p < m_data->result[resultID].fiberCount; ++p)
			{
				m_optimStepChart[chartID]->plots()[m_resultUIs[resultID].startPlotIdx + p]->setColor(getResultColor(resultID));
			}
		}
		m_optimStepChart[chartID]->update();
	}

	updateHistogramColors();
	if (m_spm->colorMode() == iAQSplom::cmByParameter)
	{
		return;
	}

	setSPMColorByResult();
	// main3DVis automatically updated through SPM
}

// Factor out as generic CSV reading class also used by iACsvIO?
bool readParameterCSV(QString const& fileName, QString const & encoding, QString const & columnSeparator, iACsvTableCreator& tblCreator, size_t resultCount)
{
	if (!QFile::exists(fileName))
	{
		DEBUG_LOG("Error loading csv file, file does not exist.");
		return false;
	}
	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		DEBUG_LOG(QString("Unable to open file '%1': %2").arg(fileName).arg(file.errorString()));
		return false;
	}
	QTextStream in(&file);
	in.setCodec(encoding.toStdString().c_str());
	auto headers = in.readLine().split(columnSeparator);
	tblCreator.initialize(headers, resultCount);
	size_t row = 0;
	while (!in.atEnd())
	{
		QString line = in.readLine();
		if (line.trimmed().isEmpty()) // skip empty lines
		{
			continue;
		}
		auto values = line.split(columnSeparator);
		tblCreator.addRow(row, values);
		++row;
	}
	return true;
}

// TODO: Refactor to use more generic data source
class iAMatrixWidget: public QWidget
{
public:
	iAMatrixWidget(std::vector<std::vector<iAResultPairInfo>>& data) :
		m_data(data),
		m_sortParam(0),
		m_dataIdx(0)
	{
		m_range[0] = m_range[1] = 0;
	}
	void setData(int idx)
	{
		m_dataIdx = idx;
		m_range[0] = std::numeric_limits<double>::max();
		m_range[1] = std::numeric_limits<double>::lowest();
		for (size_t i = 0; i < m_data.size(); ++i)
		{
			for (size_t j = 0; j < m_data[i].size(); ++j)
			{
				double value = m_data[i][j].avgDissim[m_dataIdx];
				if (value < m_range[0])
				{
					m_range[0] = value;
				}
				if (value > m_range[1])
				{
					m_range[1] = value;
				}
			}
		}
		if (m_lut.initialized())
		{
			m_lut.setRange(m_range);
		}
	}
	void setLookupTable(iALookupTable lut)
	{
		m_lut = lut;
	}
	void setParameterValues(std::vector<std::vector<double>> paramValues)
	{
		m_paramValues = paramValues;
	}
	void setSortParameter(int paramIdx)
	{
		m_sortParam = paramIdx;
		m_sortOrder = sort_indexes(m_paramValues[paramIdx]);
	}
	double const* range()
	{
		return m_range;
	}
private:
	void paintEvent(QPaintEvent* /*event*/) override
	{
		if (!m_lut.initialized())
		{
			return;
		}

		QPainter p(this);
		QFontMetrics fm = p.fontMetrics();

		int scalarBarWidth = 20;
		int scalarBarPadding = 4;

		QString minStr = dblToStringWithUnits(m_range[0]);
		QString maxStr = dblToStringWithUnits(m_range[1]);
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
		int textWidth = std::max(fm.horizontalAdvance(minStr), fm.horizontalAdvance(maxStr));
#else
		int textWidth = std::max(fm.width(minStr), fm.width(maxStr));
#endif


		int cellPixel = std::max(1,
			std::min(geometry().height() / static_cast<int>(m_data.size()),
				(geometry().width() - (3 * scalarBarPadding + scalarBarWidth + textWidth)) / static_cast<int>(m_data.size())));
		for (size_t x = 0; x < m_data.size(); ++x)
		{
			for (size_t y = 0; y < m_data[x].size(); ++y)
			{
				QRect rect(static_cast<int>(x * cellPixel), static_cast<int>(y * cellPixel), cellPixel, cellPixel);
				double value = m_data[m_sortOrder[x]][m_sortOrder[y]].avgDissim[m_dataIdx];
				QColor color = m_lut.getQColor(value);
				p.fillRect(rect, color);
			}
		}

		// Draw scalar bar (duplicated from iAQSplom!)
		QPoint topLeft(geometry().width() - (scalarBarPadding + scalarBarWidth), scalarBarPadding);

		QRect colorBarRect(topLeft.x(), topLeft.y(),
			scalarBarWidth, height() - 2*scalarBarPadding);
		QLinearGradient grad(topLeft.x(), topLeft.y(), topLeft.x(), topLeft.y() + colorBarRect.height());
		QMap<double, QColor>::iterator it;
		for (size_t i = 0; i < m_lut.numberOfValues(); ++i)
		{
			double rgba[4];
			m_lut.getTableValue(i, rgba);
			QColor color(rgba[0] * 255, rgba[1] * 255, rgba[2] * 255, rgba[3] * 255);
			double key = 1 - (static_cast<double>(i) / (m_lut.numberOfValues() - 1));
			grad.setColorAt(key, color);
		}
		p.fillRect(colorBarRect, grad);
		p.drawRect(colorBarRect);
		// Draw color bar / name of parameter used for coloring
		int colorBarTextX = topLeft.x() - (textWidth + scalarBarPadding);
		p.drawText(colorBarTextX, topLeft.y() + fm.height(), maxStr);
		p.drawText(colorBarTextX, height() - (fm.height() + scalarBarPadding), minStr);
	}
	std::vector<std::vector<iAResultPairInfo>> m_data;
	std::vector<std::vector<double>> m_paramValues;
	int m_sortParam;
	int m_dataIdx;
	iALookupTable m_lut;
	double m_range[2];
	std::vector<size_t> m_sortOrder;
};

using iADissimilarityMatrixDockContent = iAQTtoUIConnector<QWidget, Ui_DissimilarityMatrix>;

void iAFiAKErController::sensitivitySlot()
{
	QString fileName = QFileDialog::getOpenFileName(m_mainWnd, iAFiAKErController::FIAKERProjectID, m_data->folder, "Comma-Separated Values (*.csv);;");
	if (fileName.isEmpty())
	{
		return;
	}
	iACsvVectorTableCreator tblCreator;
	if (!readParameterCSV(fileName, "UTF-8", ",", tblCreator, m_data->result.size()))
	{
		return;
	}
	assert(tblCreator.table().size() > 0 && tblCreator.table()[0].size() == m_data->result.size());
	m_parameterFile = fileName;
	// compute pairwise dissimilarities between results:

	iAMeasureSelectionDlg selectMeasure;
	if (selectMeasure.exec() != QDialog::Accepted)
	{
		return;
	}
	auto measures = selectMeasure.measures();
	auto optimizationMeasureIdx = selectMeasure.optimizeMeasureIdx();

	m_dissimilarityMatrix = std::vector<std::vector<iAResultPairInfo>>(m_data->result.size(),
		std::vector<iAResultPairInfo>(m_data->result.size(),
			iAResultPairInfo(measures.size())));
	
	for (size_t resultID1 = 0; resultID1 < m_data->result.size(); ++resultID1)
	{
		auto& res1 = m_data->result[resultID1];
		auto const& mapping = *res1.mapping.data();
		double const* cxr = m_data->spmData->paramRange(mapping[iACsvConfig::CenterX]),
			* cyr = m_data->spmData->paramRange(mapping[iACsvConfig::CenterY]),
			* czr = m_data->spmData->paramRange(mapping[iACsvConfig::CenterZ]);
		double a = cxr[1] - cxr[0], b = cyr[1] - cyr[0], c = czr[1] - czr[0];
		double diagonalLength = std::sqrt(std::pow(a, 2) + std::pow(b, 2) + std::pow(c, 2));
		double const* lengthRange = m_data->spmData->paramRange(mapping[iACsvConfig::Length]);
		double maxLength = lengthRange[1] - lengthRange[0];
		for (size_t resultID2 = 0; resultID2 < m_data->result.size(); ++resultID2)
		{
			for (size_t m = 0; m < measures.size(); ++m)
			{
				m_dissimilarityMatrix[resultID1][resultID2].avgDissim[m] = 0;
			}
			if (resultID1 == resultID2)
			{
				continue;
			}
			auto& res2 = m_data->result[resultID2];
			qint64 const fiberCount = res2.table->GetNumberOfRows();
			auto& dissimilarities = m_dissimilarityMatrix[resultID1][resultID2].fiberDissim;
			dissimilarities.resize(fiberCount);
#pragma omp parallel for
			for (qint64 fiberID = 0; fiberID < fiberCount; ++fiberID)
			{
				auto it = res2.curveInfo.find(fiberID);
				// find the best-matching fibers in reference & compute difference:
				iAFiberData fiber(res2.table, fiberID, mapping, (it != res2.curveInfo.end()) ? it->second : std::vector<iAVec3f>());
				getBestMatches(fiber, mapping, res1.table, dissimilarities[fiberID], res1.curveInfo,
					diagonalLength, maxLength, measures, optimizationMeasureIdx);
				for (size_t m = 0; m < measures.size(); ++m)
				{
					m_dissimilarityMatrix[resultID1][resultID2].avgDissim[m] += dissimilarities[fiberID][m][0].dissimilarity;
				}
			}
			for (size_t m = 0; m < measures.size(); ++m)
			{
				m_dissimilarityMatrix[resultID1][resultID2].avgDissim[m] /= res2.fiberCount;
			}
		}
	}
	iADissimilarityMatrixDockContent* dissimDockContent = new iADissimilarityMatrixDockContent();

	auto measureNames = getAvailableDissimilarityMeasureNames();
	QStringList computedMeasureNames;
	for (size_t m = 0; m < measures.size(); ++m)
	{
		computedMeasureNames << measureNames[measures[m].first];
	}
	dissimDockContent->cbMeasure->addItems(computedMeasureNames);

	dissimDockContent->cbParameter->addItems(tblCreator.header());

	dissimDockContent->cbColorMap->addItems(iALUT::GetColorMapNames());

	connect(dissimDockContent->cbMeasure, SIGNAL(currentIndexChanged(int)), this, SLOT(dissimMatrixMeasureChanged(int)));
	connect(dissimDockContent->cbParameter, SIGNAL(currentIndexChanged(int)), this, SLOT(dissimMatrixParameterChanged(int)));
	connect(dissimDockContent->cbColorMap, SIGNAL(currentIndexChanged(int)), this, SLOT(dissimMatrixColorMapChanged(int)));
	m_matrixWidget = new iAMatrixWidget(m_dissimilarityMatrix);
	m_matrixWidget->setParameterValues(tblCreator.table());
	m_matrixWidget->setSortParameter(0);
	m_matrixWidget->setData(0);
	m_matrixWidget->setLookupTable(iALUT::Build(m_matrixWidget->range(), iALUT::GetColorMapNames()[0], 255, 255));
	dissimDockContent->matrix->layout()->addWidget(m_matrixWidget);
	m_views.push_back(new iADockWidgetWrapper(dissimDockContent, "Dissimilarity Matrix", "foeMatrix"));
	m_mdiChild->splitDockWidget(m_views[ResultListView], m_views[m_views.size()-1], Qt::Vertical);
	dissimMatrixMeasureChanged(0);
}

void iAFiAKErController::dissimMatrixMeasureChanged(int idx)
{
	m_matrixWidget->setData(idx);
	m_matrixWidget->update();
}

void iAFiAKErController::dissimMatrixParameterChanged(int idx)
{
	m_matrixWidget->setSortParameter(idx);
	m_matrixWidget->update();
}

void iAFiAKErController::dissimMatrixColorMapChanged(int idx)
{
	m_matrixWidget->setLookupTable(iALUT::Build(m_matrixWidget->range(), iALUT::GetColorMapNames()[idx], 255, 255));
	m_matrixWidget->update();
}

void iAFiAKErController::stackedBarColorThemeChanged(QString const & colorThemeName)
{
	addInteraction(QString("Changed stacked bar color theme to '%1'.").arg(colorThemeName));
	auto colorTheme = iAColorThemeManager::instance().theme(colorThemeName);
	m_stackedBarsHeaders->setColorTheme(colorTheme);
	for (size_t resultID = 0; resultID < m_data->result.size(); ++resultID)
	{
		m_resultUIs[resultID].stackedBars->setColorTheme(colorTheme);
	}
}

void iAFiAKErController::changeDistributionSource(int index)
{
	if (matchQualityVisActive() && m_referenceID == NoResult)
	{
		DEBUG_LOG(QString("You need to set a reference first!"));
		return;
	}
	double range[2];
	if (matchQualityVisActive())
	{
		range[0] = - 1.0;
		range[1] = 1.0;
	}
	else
	{
		range[0] = m_data->spmData->paramRange(index)[0];
		range[1] = m_data->spmData->paramRange(index)[1];
	}
	double yMax = 0;
	for (size_t resultID = 0; resultID<m_data->result.size(); ++resultID)
	{
		auto & d = m_data->result[resultID];
		auto & chart = m_resultUIs[resultID].histoChart;
		chart->clearPlots();
		chart->setXBounds(range[0], range[1]);
		if (matchQualityVisActive() && resultID != m_referenceID)
		{
			continue;
		}
		std::vector<double> fiberData(d.fiberCount);
		for (size_t fiberID = 0; fiberID < d.fiberCount; ++fiberID)
		{
			fiberData[fiberID] = matchQualityVisActive() ? m_data->avgRefFiberMatch[fiberID]
				: d.table->GetValue(fiberID, index).ToDouble();
		}
		auto histogramData = iAHistogramData::create(fiberData, HistogramBins, Continuous, range[0], range[1]);
		QSharedPointer<iAPlot> histogramPlot =
			(m_settingsView->cmbboxDistributionPlotType->currentIndex() == 0) ?
			QSharedPointer<iAPlot>(new iABarGraphPlot(histogramData, getResultColor(resultID)))
			: QSharedPointer<iAPlot>(new iALinePlot(histogramData, getResultColor(resultID)));
		chart->addPlot(histogramPlot);
		if (histogramData->yBounds()[1] > yMax)
		{
			yMax = histogramData->yBounds()[1];
		}
	}
	for (size_t resultID = 0; resultID < m_data->result.size(); ++resultID)
	{
		m_resultUIs[resultID].histoChart->setYBounds(0, yMax);
	}
	updateRefDistPlots();
	if (m_colorByDistribution->isChecked())
	{
		colorByDistrToggled();
	}
	updateHistogramColors();
}

void iAFiAKErController::updateHistogramColors()
{
	double range[2] = { 0.0, static_cast<double>(HistogramBins) };
	auto lut = m_colorByDistribution->isChecked() ?
		QSharedPointer<iALookupTable>(new iALookupTable(iALUT::Build(range, m_colorByThemeName, 255, 1)))
		: QSharedPointer<iALookupTable>();
	for (size_t resultID = 0; resultID < m_data->result.size(); ++resultID)
	{
		auto & chart = m_resultUIs[resultID].histoChart;
		if (chart->plots().size() > 0)
		{
			if (dynamic_cast<iABarGraphPlot*>(chart->plots()[0].data()))
			{
				dynamic_cast<iABarGraphPlot*>(chart->plots()[0].data())->setLookupTable(lut);
			}
			if (!lut)
			{
				chart->plots()[0]->setColor(getResultColor(resultID));
			}
		}
		if (chart->plots().size() > 1)
		{
			chart->plots()[1]->setColor(getResultColor(m_referenceID));
		}
		chart->update();
	}
}

void iAFiAKErController::updateRefDistPlots()
{
	for (size_t resultID = 0; resultID < m_data->result.size(); ++resultID)
	{
		auto & chart = m_resultUIs[resultID].histoChart;
		if (chart->plots().size() > 1)
		{
			chart->removePlot(chart->plots()[1]);
		}
		if (m_referenceID != NoResult && resultID != m_referenceID && !matchQualityVisActive() && m_settingsView->cbShowReferenceDistribution->isChecked())
		{
			QColor refColor = getResultColor(m_referenceID);
			refColor.setAlpha(DistributionRefAlpha);
			QSharedPointer<iAPlotData> refPlotData = m_resultUIs[m_referenceID].histoChart->plots()[0]->data();
			QSharedPointer<iAPlot> refPlot =
				(m_settingsView->cmbboxDistributionPlotType->currentIndex() == 0) ?
				QSharedPointer<iAPlot>(new iABarGraphPlot(refPlotData, refColor))
				: QSharedPointer<iAPlot>(new iALinePlot(refPlotData, refColor));
			chart->addPlot(refPlot);
		}
		chart->update();
	}
}

void iAFiAKErController::colorByDistrToggled()
{
	addInteraction(QString("Toggled color by distribution %1.").arg(m_colorByDistribution->isChecked()?"on":"off"));
	if (m_colorByDistribution->isChecked())
	{
		size_t colorLookupParam = m_distributionChoice->currentIndex();
		if (matchQualityVisActive())
		{
			// set all currently shown main visualizations back to their result color
			for (size_t resultID = 0; resultID < m_resultUIs.size(); ++resultID)
			{
				if (resultID == m_referenceID)
				{
					continue;
				}
				auto mainVis = m_resultUIs[resultID].main3DVis;
				if (mainVis->visible())
				{
					mainVis->setColor(getResultColor(resultID));
				}
			}
			setSPMColorByResult();
			showSpatialOverview();
		}
		else
		{   // this triggers also spmLookupTableChanged (which updates the 3D views)
			m_spm->setColorParam(colorLookupParam);
			m_spm->rangeFromParameter();
		}
	}
	else
	{
		setSPMColorByResult();
	}
	updateHistogramColors();
}

void iAFiAKErController::exportDissimilarities()
{
	if (m_referenceID == NoResult)
	{
		DEBUG_LOG("No reference set, therefore there are no dissimilarities to export!");
		return;
	}
	QString fileName = QFileDialog::getSaveFileName(m_mainWnd, iAFiAKErController::FIAKERProjectID, m_data->folder, "Comma-Separated Values (*.csv);;");
	if (fileName.isEmpty())
	{
		return;
	}
	QFile outFile(fileName);
	if (!outFile.open(QIODevice::WriteOnly))
	{
		DEBUG_LOG(QString("Cannot open file %1 for writing!").arg(fileName));
		return;
	}
	QTextStream out(&outFile);
	out << "ResultID";
	auto measureNames = getAvailableDissimilarityMeasureNames();
	for (auto measureID: m_data->m_measures)
	{
		out << "," << measureNames[measureID];
	}
	out << endl;
	QFileInfo fi(fileName);
	for (size_t resultID = 0; resultID < m_data->result.size(); ++resultID)
	{
		out << resultID;
		auto& r = m_data->result[resultID];
		auto& avgMeasure = r.avgDifference;
		if (resultID == m_referenceID)
		{
			out << ",REFERENCE";
		}
		else
		{
			for (int m = avgMeasure.size() - m_data->m_measures.size();
				m >= 0 && m < avgMeasure.size(); ++m)
			{
				out << "," << avgMeasure[m];
			}
		}
		out << endl;

		if (resultID == m_referenceID)
		{
			continue;
		}
		QString resultFileName = fi.absolutePath() + "/" + fi.baseName() + "-" + QFileInfo(r.fileName).baseName() + ".csv";
		QFile resultOutFile(resultFileName);
		if (!resultOutFile.open(QIODevice::WriteOnly))
		{
			DEBUG_LOG(QString("Cannot open file %1 for writing!").arg(fileName));
			return;
		}
		const int NumOfMatchesToWrite = 3;
		QTextStream resultOut(&resultOutFile);
		resultOut << "LabelID";
		for (auto measureID: m_data->m_measures)
		{
			for (int i = 0; i < NumOfMatchesToWrite; ++i)
			{
				resultOut << "," << measureNames[measureID] << QString(" Fiber ID Match %1").arg(i)
					<< "," << measureNames[measureID] << QString(" Dissimilarity %1").arg(i);
			}
		}
		resultOut << endl;
		for (int fiberID = 0; fiberID < r.refDiffFiber.size(); ++fiberID)
		{
			auto& f = r.refDiffFiber[fiberID].dist;
			resultOut << fiberID + 1;
			for (int m = 0; m < f.size(); ++m)
			{
				for (int i = 0; i < NumOfMatchesToWrite; ++i)
				{
					resultOut << "," << f[m][i].index << "," << f[m][i].dissimilarity;
				}
			}
			resultOut << endl;
		}
		resultOutFile.close();
	}
	outFile.close();
}

void iAFiAKErController::sortByCurrentWeighting()
{
	std::vector<std::pair<size_t, double>> resultWeights;
	for (size_t resultID = 0; resultID < m_data->result.size(); ++resultID)
	{
		auto& ui = m_resultUIs[resultID];
		resultWeights.push_back(std::make_pair(resultID, ui.stackedBars->weightedSum()));
	}
	std::sort(resultWeights.begin(), resultWeights.end(),
		[](std::pair<size_t, double> const& a, std::pair<size_t, double> const& b)
		{
			return a.second < b.second;
		});
	m_resultListSorting.clear();
	for (size_t itemNumber = 0; itemNumber < resultWeights.size(); ++itemNumber)
	{
		m_resultListSorting.insert(resultWeights[itemNumber].first, static_cast<int>(itemNumber));
	}
	updateResultList();
}

QColor iAFiAKErController::getResultColor(size_t resultID)
{
	QColor color = m_resultColorTheme->color( resultID % m_resultColorTheme->size() );
	color.setAlpha(SelectionOpacity);
	return color;
}

namespace
{
	bool resultSelected(std::vector<iAFiberCharUIData> const & uiCollection, size_t resultID)
	{
		return (uiCollection[resultID].main3DVis->visible());
	}
	bool noResultSelected(std::vector<iAFiberCharUIData> const & uiCollection)
	{
		for (size_t i = 0; i < uiCollection.size(); ++i)
		{
			if (resultSelected(uiCollection, i))
			{
				return false;
			}
		}
		return true;
	}
	bool anyOtherResultSelected(std::vector<iAFiberCharUIData> const & uiCollection, size_t resultID)
	{
		for (size_t i = 0; i < uiCollection.size(); ++i)
		{
			if (resultSelected(uiCollection, i) && resultID != i)
			{
				return true;
			}
		}
		return false;
	}
}

void iAFiAKErController::toggleOptimStepChart(size_t chartID, bool visible)
{
	if (!visible)
	{
		if (!m_optimStepChart[chartID])
		{
			DEBUG_LOG(QString("Step chart %1 toggled invisible, but not created yet.").arg(chartID));
			return;
		}
		m_optimStepChart[chartID]->setVisible(false);
		return;
	}
	if (!m_optimStepChart[chartID])
	{
		if (chartID < m_chartCount-1 && m_referenceID == NoResult)
		{
			DEBUG_LOG(QString("You need to set a reference first!"));
			return;
		}
		m_optimStepChart[chartID] = new iAChartWidget(nullptr, "Optimization Step", diffName(chartID));
		m_optimStepChart[chartID]->setDrawXAxisAtZero(true);
		size_t plotsBefore = 0, curIdx = 0;
		while (curIdx < chartID)
		{  // TODO: check invisible plots?
			if (m_optimStepChart[curIdx])
			{
				++plotsBefore;
			}
			++curIdx;
		}
		m_optimChartLayout->insertWidget(plotsBefore, m_optimStepChart[chartID]);
		m_optimStepChart[chartID]->setMinimumHeight(100);
		m_optimStepChart[chartID]->setSelectionMode(iAChartWidget::SelectPlot);
		m_optimStepChart[chartID]->addXMarker(m_data->optimStepMax -1, OptimStepMarkerColor);
		for (size_t resultID=0; resultID<m_data->result.size(); ++resultID)
		{
			auto & d = m_data->result[resultID];
			if (m_resultUIs[resultID].startPlotIdx == NoPlotsIdx)
			{
				continue;
			}
			for (size_t fiberID = 0; fiberID < d.fiberCount; ++fiberID)
			{
				QSharedPointer<iAVectorPlotData> plotData;
				if (chartID < m_chartCount - 1)
				{
					if (chartID < static_cast<size_t>(d.refDiffFiber[fiberID].diff.size()))
					{
						plotData = QSharedPointer<iAVectorPlotData>(new iAVectorPlotData(d.refDiffFiber[fiberID].diff[chartID].step));
					}
					else
					{
						DEBUG_LOG("Differences for this measure not computed (yet).");
						return;
					}
				}
				else
				{
					plotData = QSharedPointer<iAVectorPlotData>(new iAVectorPlotData(d.projectionError[fiberID]));
				}
				plotData->setXDataType(Discrete);
				m_optimStepChart[chartID]->addPlot(QSharedPointer<iALinePlot>(new iALinePlot(plotData, getResultColor(resultID))));
			}
		}
		connect(m_optimStepChart[chartID], &iAChartWidget::plotsSelected,
				this, &iAFiAKErController::selectionOptimStepChartChanged);
	}
	m_optimStepChart[chartID]->setVisible(true);
	m_optimStepChart[chartID]->clearMarkers();
	m_optimStepChart[chartID]->addXMarker(m_optimStepSlider->value(), OptimStepMarkerColor);

	bool allVisible = noResultSelected(m_resultUIs);
	for (size_t resultID=0; resultID<m_data->result.size(); ++resultID)
	{
		if (m_resultUIs[resultID].startPlotIdx == NoPlotsIdx)
		{
			continue;
		}
		for (size_t p = 0; p < m_data->result[resultID].fiberCount; ++p)
		{
			if (p < m_optimStepChart[chartID]->plots().size())
			{
				m_optimStepChart[chartID]->plots()[m_resultUIs[resultID].startPlotIdx + p]
					->setVisible(allVisible || resultSelected(m_resultUIs, resultID));
			}
			else
			{
				DEBUG_LOG("Tried to show/hide unavailable plot.");
				return;
			}
		}
	}
	m_optimStepChart[chartID]->update();
	showSelectionInPlot(chartID);
}

void iAFiAKErController::addInteraction(QString const & interaction)
{
	m_interactionProtocolModel->invisibleRootItem()->appendRow(new QStandardItem(interaction));
}

void iAFiAKErController::toggleVis(int state)
{
	size_t resultID = QObject::sender()->property("resultID").toULongLong();
	addInteraction(QString("Toggle visibility of %1 to %2.").arg(resultName(resultID)).arg(state?"on":"off"));
	showMainVis(resultID, state);
}

void iAFiAKErController::showMainVis(size_t resultID, int state)
{
	auto & d = m_data->result[resultID];
	auto & ui = m_resultUIs[resultID];
	if (state == Qt::Checked)
	{
		ui.main3DVis->setSelectionOpacity(SelectionOpacity);
		ui.main3DVis->setContextOpacity(ContextOpacity);
		ui.main3DVis->setShowWireFrame(m_showWireFrame);
		ui.main3DVis->setShowLines(m_showLines);
		setClippingPlanes(ui.main3DVis);
		auto vis = dynamic_cast<iA3DCylinderObjectVis*>(ui.main3DVis.data());
		if (vis)
		{
			vis->setDiameterFactor(DiameterFactor);
			vis->setContextDiameterFactor(ContextDiameterFactor);
		}
		if (matchQualityVisActive())
		{
			showSpatialOverview();
		}
		else if (m_spm->colorMode() == iAQSplom::cmByParameter)
		{
			if (vis)
			{
				vis->setLookupTable(m_spm->lookupTable(), m_spm->colorLookupParam());
				vis->updateColorSelectionRendering();
			}
		}
		else
		{
			ui.main3DVis->setColor(getResultColor(resultID));
		}
		if (ui.startPlotIdx != NoPlotsIdx)
		{
			if (!anyOtherResultSelected(m_resultUIs, resultID))
			{
				for (size_t c = 0; c < m_chartCount; ++c)
				{
					if (m_optimStepChart[c] && m_optimStepChart[c]->isVisible())
					{
						for (size_t p = 0; p < m_optimStepChart[c]->plots().size(); ++p)
						{
							m_optimStepChart[c]->plots()[p]->setVisible(false);
						}
					}
				}
			}
			for (size_t c = 0; c < m_chartCount; ++c)
			{
				if (m_optimStepChart[c] && m_optimStepChart[c]->isVisible())
				{
					for (size_t p = 0; p < d.fiberCount; ++p)
					{
						m_optimStepChart[c]->plots()[ui.startPlotIdx + p]->setVisible(true);
					}
				}
			}
		}

		bool anythingSelected = isAnythingSelected();
		if (anythingSelected)
		{
			ui.main3DVis->setSelection(m_selection[resultID], anythingSelected);
		}
		if ((m_data->objectType == iACsvConfig::Cylinders || m_data->objectType == iACsvConfig::Lines) &&
			d.stepData != iAFiberCharData::NoStepData &&
			m_useStepData)
		{
			vis->updateValues(d.stepValues[
				std::min(d.stepValues.size() - 1, static_cast<size_t>(m_optimStepSlider->value()))],
				d.stepData);
		}
		ui.main3DVis->show();
		if (!m_cameraInitialized)
		{
			m_ren->ResetCamera();
			m_cameraInitialized = true;
		}
		m_style->addInput(resultID, ui.main3DVis->getPolyData(), ui.main3DVis->getActor() );
		m_spm->addFilter(m_data->m_resultIDColumn, resultID);
	}
	else
	{
		if (ui.startPlotIdx != NoPlotsIdx)
		{
			if (anyOtherResultSelected(m_resultUIs, resultID))
			{
				for (size_t c = 0; c < m_chartCount; ++c)
				{
					if (m_optimStepChart[c] && m_optimStepChart[c]->isVisible())
					{
						for (size_t p = 0; p < d.fiberCount; ++p)
						{
							if (ui.startPlotIdx + p >= m_optimStepChart[c]->plots().size())
							{
								DEBUG_LOG(QString("Invalid chart access: access to plot %1, but only has %2")
									.arg(ui.startPlotIdx + p)
									.arg(m_optimStepChart[c]->plots().size()));
							}
							else
							{
								m_optimStepChart[c]->plots()[ui.startPlotIdx + p]->setVisible(false);
							}
						}
					}
				}
			}
			else // nothing selected, show everything
			{
				for (size_t c = 0; c < m_chartCount; ++c)
				{
					if (m_optimStepChart[c] && m_optimStepChart[c]->isVisible())
					{
						for (size_t p = 0; p < m_optimStepChart[c]->plots().size(); ++p)
						{
							m_optimStepChart[c]->plots()[p]->setVisible(true);
						}
					}
				}
			}
		}
		ui.main3DVis->hide();
		m_style->removeInput(resultID);
		m_spm->removeFilter(m_data->m_resultIDColumn, resultID);
	}
	for (size_t c = 0; c < m_chartCount; ++c)
	{
		if (m_optimStepChart[c] && m_optimStepChart[c]->isVisible())
		{
			m_optimStepChart[c]->update();
		}
	}
	changeReferenceDisplay();
	update3D();
}

void iAFiAKErController::toggleBoundingBox(int state)
{
	size_t resultID = QObject::sender()->property("resultID").toULongLong();
	addInteraction(QString("Toggle bounding box of result %1 to %2.")
		.arg(resultName(resultID)).arg(state ? "on" : "off"));
	auto & ui = m_resultUIs[resultID];
	if (state == Qt::Checked)
	{
		ui.main3DVis->showBoundingBox();
		if (!m_cameraInitialized)
		{
			m_ren->ResetCamera();
			m_cameraInitialized = true;
		}
	}
	else
	{
		ui.main3DVis->hideBoundingBox();
	}
}

void iAFiAKErController::getResultFiberIDFromSpmID(size_t spmID, size_t & resultID, size_t & fiberID)
{
	size_t curStart = 0;
	resultID = 0;
	fiberID = 0;
	while (spmID >= curStart + m_data->result[resultID].fiberCount && resultID < m_data->result.size())
	{
		curStart += m_data->result[resultID].fiberCount;
		++resultID;
	}
	if (resultID == m_data->result.size())
	{
		DEBUG_LOG(QString("Invalid index in SPM: %1").arg(spmID));
		return;
	}
	fiberID = spmID - curStart;
}

std::vector<std::vector<size_t> > & iAFiAKErController::selection()
{
	return m_selection;
}

void iAFiAKErController::clearSelection()
{
	for (size_t resultID = 0; resultID < m_selection.size(); ++resultID)
	{
		m_selection[resultID].clear();
	}
}

void iAFiAKErController::sortSelection(QString const & source)
{
	for (size_t resultID = 0; resultID < m_selection.size(); ++resultID)
	{
		std::sort(m_selection[resultID].begin(), m_selection[resultID].end());
	}
	newSelection(source);
}

void iAFiAKErController::newSelection(QString const & source)
{
	size_t selSize = selectionSize();
	if (selSize == 0 || (m_selections.size() > 0 && m_selection == m_selections[m_selections.size() - 1]))
	{
		return;
	}
	size_t resultCount = 0;
	for (size_t resultID = 0; resultID < m_selection.size(); ++resultID)
	{
		resultCount += (m_selection[resultID].size() > 0) ? 1 : 0;
	}
	m_selections.push_back(m_selection);
	m_selectionListModel->appendRow(new QStandardItem(QString("%1 fibers in %2 results (%3)")
		.arg(selSize).arg(resultCount).arg(source)));
	showSelectionDetail();
}

size_t iAFiAKErController::selectionSize() const
{
	size_t selectionSize = 0;
	for (size_t resultID = 0; resultID < m_selection.size(); ++resultID)
	{
		selectionSize += m_selection[resultID].size();
	}
	return selectionSize;
}

void iAFiAKErController::showSelectionInPlots()
{
	for (size_t chartID = 0; chartID < m_chartCount; ++chartID)
	{
		showSelectionInPlot(chartID);
	}
}

void iAFiAKErController::showSelectionInPlot(int chartID)
{
	auto chart = m_optimStepChart[chartID];
	if (!chart || !chart->isVisible())
	{
		return;
	}
	for (size_t resultID = 0; resultID < m_data->result.size(); ++resultID)
	{
		if (m_resultUIs[resultID].startPlotIdx != NoPlotsIdx)
		{
			size_t curSelIdx = 0;
			QColor color(getResultColor(resultID));
			if (isAnythingSelected())
			{
				color.setAlpha(ContextOpacity);
			}
			for (size_t fiberID=0; fiberID < m_data->result[resultID].fiberCount; ++fiberID)
			{
				if (m_resultUIs[resultID].startPlotIdx + fiberID > chart->plots().size())
				{
					break;
				}
				auto plot = dynamic_cast<iALinePlot*>(chart->plots()[m_resultUIs[resultID].startPlotIdx + fiberID].data());
				if (curSelIdx < m_selection[resultID].size() && fiberID == m_selection[resultID][curSelIdx])
				{
					plot->setLineWidth(2);
					plot->setColor(SelectionColor);
					++curSelIdx;
				}
				else
				{
					plot->setLineWidth(1);
					plot->setColor(color);
				}
			}
		}
	}
	chart->update();
}

bool iAFiAKErController::isAnythingSelected() const
{
	for (size_t resultID = 0; resultID < m_data->result.size(); ++resultID)
	{
		if (m_selection[resultID].size() > 0)
		{
			return true;
		}
	}
	return false;
}

void iAFiAKErController::showSelectionIn3DViews()
{
	bool anythingSelected = isAnythingSelected();
	m_showReferenceWidget->setVisible(anythingSelected);
	for (size_t resultID = 0; resultID < m_resultUIs.size(); ++resultID)
	{
		auto& vis = m_resultUIs[resultID];
		if (vis.main3DVis->visible())
		{
			vis.main3DVis->setSelection(m_selection[resultID], anythingSelected);
		}
	}
	// TODO: prevent multiple render window / widget updates?
}

void iAFiAKErController::showSelectionInSPM()
{
	std::vector<size_t> spmSelection;
	spmSelection.reserve(selectionSize());
	size_t spmIDStart = 0;
	for (size_t resultID = 0; resultID<m_data->result.size(); ++resultID)
	{
		for (size_t fiberID = 0; fiberID < m_selection[resultID].size(); ++fiberID)
		{
			size_t spmID = spmIDStart + m_selection[resultID][fiberID];
			spmSelection.push_back(spmID);
		}
		spmIDStart += m_data->result[resultID].fiberCount;
	}
	m_spm->setSelection(spmSelection);
}

void iAFiAKErController::selection3DChanged()
{
	addInteraction(QString("Selected %1 fibers in 3D view.").arg(selectionSize()));
	sortSelection("3D view");
	showSelectionIn3DViews();
	showSelectionInPlots();
	showSelectionInSPM();
	changeReferenceDisplay();
	updateFiberContext();
	if (isAnythingSelected() && !m_views[SelectionView]->isVisible())
	{
		m_views[SelectionView]->show();
	}
}

void iAFiAKErController::selectionSPMChanged(std::vector<size_t> const & selection)
{
	addInteraction(QString("Selected %1 fibers in scatter plot matrix.").arg(selection.size()));
	// map from SPM index to (resultID, fiberID) pairs
	clearSelection();
	size_t resultID, fiberID;
	for (size_t spmID: selection)
	{
		getResultFiberIDFromSpmID(spmID, resultID, fiberID);
		m_selection[resultID].push_back(fiberID);
	}
	sortSelection("SPM");
	showSelectionIn3DViews();
	showSelectionInPlots();
	changeReferenceDisplay();
	updateFiberContext();
	if (isAnythingSelected() && !m_views[SelectionView]->isVisible())
	{
		m_views[SelectionView]->show();
	}
}

void iAFiAKErController::selectionOptimStepChartChanged(std::vector<size_t> const & selection)
{
	addInteraction(QString("Selected %1 fibers in optimization step chart.").arg(selection.size()));
	size_t curSelectionIndex = 0;
	clearSelection();
	// map from plot IDs to (resultID, fiberID) pairs
	for (size_t resultID=0; resultID<m_resultUIs.size() && curSelectionIndex < selection.size(); ++resultID)
	{
		if (m_resultUIs[resultID].startPlotIdx != NoPlotsIdx)
		{
			while (curSelectionIndex < selection.size() &&
				   selection[curSelectionIndex] <
				   (m_resultUIs[resultID].startPlotIdx + m_data->result[resultID].fiberCount) )
			{
				size_t inResultFiberIdx = selection[curSelectionIndex] - m_resultUIs[resultID].startPlotIdx;
				m_selection[resultID].push_back(inResultFiberIdx);
				++curSelectionIndex;
			}
		}
	}
	sortSelection("Chart");
	showSelectionInPlots();
	showSelectionIn3DViews();
	showSelectionInSPM();
	changeReferenceDisplay();
	updateFiberContext();
	if (isAnythingSelected() && !m_views[SelectionView]->isVisible())
	{
		m_views[SelectionView]->show();
	}
}

void iAFiAKErController::miniMouseEvent(QMouseEvent* ev)
{
	if (ev->buttons() == Qt::RightButton && ev->type() == QEvent::MouseButtonPress)
	{
		size_t resultID = QObject::sender()->property("resultID").toULongLong();
		addInteraction(QString("Started FiberScout for %1.").arg(resultName(resultID)));
		MdiChild* newChild = m_mainWnd->createMdiChild(false);
		newChild->show();
		// wait a bit to make sure MdiChild is shown and all initialization is done
		// TODO: Replace by connection to a signal which is emitted when MdiChild initialization done
		QTimer::singleShot(1000, [this, resultID, newChild] { startFeatureScout(resultID, newChild); });
		ev->accept();  // not sure if this helps, sometimes still the context menu seems to pop up
	}
}

void iAFiAKErController::startFeatureScout(int resultID, MdiChild* newChild)
{
	iACsvConfig config(m_config);
	config.fileName = m_data->result[resultID].fileName;
	config.curvedFiberFileName = m_data->result[resultID].curvedFileName;
	iAFeatureScoutModuleInterface * featureScout = m_mainWnd->getModuleDispatcher().GetModule<iAFeatureScoutModuleInterface>();
	featureScout->LoadFeatureScout(config, newChild);
	//newChild->loadLayout("FeatureScout");
}

void iAFiAKErController::optimStepSliderChanged(int optimStep)
{
	addInteraction(QString("Set optimization step slider to step %1.").arg(optimStep));
	setOptimStep(optimStep);
}

void iAFiAKErController::setOptimStep(int optimStep)
{
	m_currentOptimStepLabel->setText(QString::number(optimStep));
	for (size_t chartID= 0; chartID < m_chartCount; ++chartID)
	{
		auto chart = m_optimStepChart[chartID];
		if (!chart || !chart->isVisible())
		{
			continue;
		}
		chart->clearMarkers();
		chart->addXMarker(optimStep, OptimStepMarkerColor);
		chart->update();
		for (size_t resultID = 0; resultID < m_data->result.size(); ++resultID)
		{
			auto main3DVis = m_resultUIs[resultID].main3DVis;
			if (main3DVis->visible() &&
				m_data->objectType == iACsvConfig::Cylinders &&
				m_data->result[resultID].stepData != iAFiberCharData::NoStepData)
			{
				auto & stepValues = m_data->result[resultID].stepValues;
				auto vis = dynamic_cast<iA3DCylinderObjectVis*>(main3DVis.data());
				vis->updateValues(stepValues[std::min(static_cast<size_t>(optimStep), stepValues.size() - 1)],
					m_data->result[resultID].stepData);
			}
		}
	}
	changeReferenceDisplay();
}

void iAFiAKErController::mainOpacityChanged(int opacity)
{
	addInteraction(QString("Set main opacity to %1.").arg(opacity));
	m_settingsView->lbOpacityDefaultValue->setText(QString::number(opacity, 'f', 2));
	SelectionOpacity = opacity;
	visitAllVisibleVis([](QSharedPointer<iA3DColoredPolyObjectVis> vis, size_t /*resultID*/)
	{
		vis->setSelectionOpacity(SelectionOpacity);
		vis->updateColorSelectionRendering();
	});
}

void iAFiAKErController::contextOpacityChanged(int opacity)
{
	addInteraction(QString("Set context opacity to %1.").arg(opacity));
	m_settingsView->lbOpacityContextValue->setText(QString::number(opacity, 'f', 2));
	ContextOpacity = opacity;
	visitAllVisibleVis([](QSharedPointer<iA3DColoredPolyObjectVis> vis, size_t /*resultID*/)
	{
		vis->setContextOpacity(ContextOpacity);
		vis->updateColorSelectionRendering();
	});
	showSelectionInPlots();
}

void iAFiAKErController::diameterFactorChanged(int diameterFactorInt)
{
	if (m_data->objectType != iACsvConfig::Cylinders)
	{
		return;
	}
	DiameterFactor = m_diameterFactorMapper->dstToSrc(diameterFactorInt);
	addInteraction(QString("Set diameter modification factor to %1.").arg(DiameterFactor));
	m_settingsView->lbDiameterFactorDefaultValue->setText(QString::number(DiameterFactor, 'f', 2));
	visitAllVisibleVis([](QSharedPointer<iA3DColoredPolyObjectVis> vis, size_t /*resultID*/)
	{
		(dynamic_cast<iA3DCylinderObjectVis*>(vis.data()))->setDiameterFactor(DiameterFactor);
	});
}

void iAFiAKErController::contextDiameterFactorChanged(int contextDiameterFactorInt)
{
	if (m_data->objectType != iACsvConfig::Cylinders)
	{
		return;
	}
	ContextDiameterFactor = m_diameterFactorMapper->dstToSrc(contextDiameterFactorInt);
	addInteraction(QString("Set context diameter modification factor to %1.").arg(ContextDiameterFactor));
	m_settingsView->lbDiameterFactorContextValue->setText(QString::number(ContextDiameterFactor, 'f', 2));
	visitAllVisibleVis([](QSharedPointer<iA3DColoredPolyObjectVis> vis, size_t /*resultID*/)
	{
		(dynamic_cast<iA3DCylinderObjectVis*>(vis.data()))->setContextDiameterFactor(ContextDiameterFactor);
	});
}

void iAFiAKErController::showFiberContextChanged(int newState)
{
	m_showFiberContext = (newState == Qt::Checked);
	updateFiberContext();
}

void iAFiAKErController::mergeFiberContextBoxesChanged(int newState)
{
	m_mergeContextBoxes = (newState == Qt::Checked);
	updateFiberContext();
}

void iAFiAKErController::visitAllVisibleVis(std::function<void(QSharedPointer<iA3DColoredPolyObjectVis>, size_t)> func)
{
	for (size_t resultID = 0; resultID < m_resultUIs.size(); ++resultID)
	{
		auto& vis = m_resultUIs[resultID];
		if (vis.mini3DVis)
		{
			func(vis.mini3DVis, resultID);
		}
		if (vis.main3DVis->visible())
		{
			func(vis.main3DVis, resultID);
		}
	}
}

void iAFiAKErController::showWireFrameChanged(int newState)
{
	m_showWireFrame = (newState == Qt::Checked);
	visitAllVisibleVis([this](QSharedPointer<iA3DColoredPolyObjectVis> vis, size_t /*resultID*/)
	{
		vis->setShowWireFrame(m_showWireFrame);
	});
}

void iAFiAKErController::showLinesChanged(int newState)
{
	m_showLines = (newState == Qt::Checked);
	visitAllVisibleVis([this](QSharedPointer<iA3DColoredPolyObjectVis> vis, size_t /*resultID*/)
	{
		vis->setShowLines(m_showLines);
	});
}

void iAFiAKErController::showBoundingBoxChanged(int newState)
{
	if (newState == Qt::Checked)
	{
		m_ren->AddActor(m_customBoundingBoxActor);
		updateBoundingBox();
	}
	else
	{
		m_ren->RemoveActor(m_customBoundingBoxActor);
		update3D();
	}
}

void iAFiAKErController::updateBoundingBox()
{
	if (!m_ren->HasViewProp(m_customBoundingBoxActor))
	{
		return;
	}
	// TODO: move to function also called when edit fields change
	double newBounds[6];
	for (int i = 0; i < 3; ++i)
	{                                             // todo: error checking
		bool ok;
		newBounds[i * 2] = m_teBoundingBox[i]->text().toDouble(&ok);
		if (!ok)
		{
			DEBUG_LOG(QString("Invalid bounding box value: %1").arg(m_teBoundingBox[i]->text()))
		}
		newBounds[i * 2 + 1] = m_teBoundingBox[i + 3]->text().toDouble(&ok);
		if (!ok)
		{
			DEBUG_LOG(QString("Invalid bounding box value: %1").arg(m_teBoundingBox[i]->text()))
		}
	}
	m_customBoundingBoxSource->SetBounds(newBounds);
	m_customBoundingBoxMapper->Update();

	update3D();
}

void iAFiAKErController::contextSpacingChanged(double value)
{
	m_contextSpacing = value;
	updateFiberContext();
}

namespace
{
	vtkSmartPointer<vtkActor> getCubeActor(iAVec3T<double> const & start, iAVec3T<double> const & end)
	{
		auto cube = vtkSmartPointer<vtkCubeSource>::New();
		cube->SetXLength(abs(start[0] - end[0]));
		cube->SetYLength(abs(start[1] - end[1]));
		cube->SetZLength(abs(start[2] - end[2]));
		auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
		mapper->SetInputConnection(cube->GetOutputPort());
		auto actor = vtkSmartPointer<vtkActor>::New();
		auto pos = (start + end) / 2;
		actor->SetPosition( pos.data() );
		actor->GetProperty()->SetRepresentationToWireframe();
		actor->SetMapper(mapper);
		return actor;
	}
}

void iAFiAKErController::updateFiberContext()
{
	for (auto actor : m_contextActors)
	{
#if VTK_MAJOR_VERSION < 9
		m_main3DWidget->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->RemoveActor(actor);
#else
		m_main3DWidget->renderWindow()->GetRenderers()->GetFirstRenderer()->RemoveActor(actor);
#endif
	}
	m_contextActors.clear();
	if (m_showFiberContext)
	{
		iAVec3T<double> minCoord(std::numeric_limits<double>::max()), maxCoord(std::numeric_limits<double>::lowest());
		for (size_t resultID = 0; resultID < m_data->result.size(); ++resultID)
		{
			auto & d = m_data->result[resultID];
			for (size_t selectionID = 0; selectionID < m_selection[resultID].size(); ++selectionID)
			{
				size_t fiberID = m_selection[resultID][selectionID];
				double diameter = d.table->GetValue(fiberID, d.mapping->value(iACsvConfig::Diameter)).ToFloat();
				double radius = diameter / 2;
				if (!m_mergeContextBoxes)
				{
					minCoord = std::numeric_limits<double>::max();
					maxCoord = std::numeric_limits<double>::lowest();
				}
				for (int i = 0; i < 3; ++i)
				{
					double startI = d.table->GetValue(fiberID, d.mapping->value(iACsvConfig::StartX + i)).ToFloat();
					double endI = d.table->GetValue(fiberID, d.mapping->value(iACsvConfig::EndX + i)).ToFloat();

					if ((startI - radius - m_contextSpacing) < minCoord[i])
					{
						minCoord[i] = startI - radius - m_contextSpacing;
					}
					if ((endI - radius - m_contextSpacing) < minCoord[i])
					{
						minCoord[i] = endI - radius - m_contextSpacing;
					}

					if ((startI + radius + m_contextSpacing) > maxCoord[i])
					{
						maxCoord[i] = startI + radius + m_contextSpacing;
					}
					if ((endI + radius + m_contextSpacing) > maxCoord[i])
					{
						maxCoord[i] = endI + radius + m_contextSpacing;
					}
				}
				if (!m_mergeContextBoxes)
				{
					auto actor = getCubeActor(minCoord, maxCoord);
#if VTK_MAJOR_VERSION < 9
					m_main3DWidget->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(actor);
#else
					m_main3DWidget->renderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(actor);
#endif
					m_contextActors.push_back(actor);
				}
			}
		}
		if (m_mergeContextBoxes)
		{
			auto actor = getCubeActor(minCoord, maxCoord);
#if VTK_MAJOR_VERSION < 9
			m_main3DWidget->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(actor);
#else
			m_main3DWidget->renderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(actor);
#endif
			m_contextActors.push_back(actor);
		}
	}
	update3D();
}

namespace
{
	void setResultBackground(iAFiberCharUIData & ui, QColor const & color)
	{
		ui.nameActions->setBackgroundColor(color);
		ui.topFiller->setStyleSheet("background-color: " + color.name());
		ui.bottomFiller->setStyleSheet("background-color: " + color.name());
		if (ui.previewWidget && ui.vtkWidget)
		{
			ui.previewWidget->setBackgroundColor(color);
#if VTK_MAJOR_VERSION < 9
			ui.vtkWidget->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->SetBackground(
#else
			ui.vtkWidget->renderWindow()->GetRenderers()->GetFirstRenderer()->SetBackground(
#endif
				color.redF(), color.greenF(), color.blueF());
			ui.vtkWidget->update();
		}
		ui.stackedBars->setBackgroundColor(color);
		ui.histoChart->setBackgroundColor(color);
	}
}

void iAFiAKErController::referenceToggled()
{
	if (m_refDistCompute)
	{
		DEBUG_LOG("Another reference computation is currently running, please let that finish first!");
		return;
	}
	size_t referenceID = QObject::sender()->property("resultID").toULongLong();

	iAMeasureSelectionDlg measureDlg;
	if (measureDlg.exec() != QDialog::Accepted)
	{
		return;
	}
	setReference(referenceID, measureDlg.measures(), measureDlg.optimizeMeasureIdx(), measureDlg.bestMeasureIdx());
}

void iAFiAKErController::setReference(size_t referenceID, std::vector<std::pair<int, bool>> measures, int optimizationMeasure, int bestMeasure)
{
	if (referenceID == m_referenceID)
	{
		DEBUG_LOG(QString("The selected result (%1) is already set as reference!").arg(referenceID));
		return;
	}
	if (m_referenceID != NoResult)
	{
		if (QMessageBox::question(m_mainWnd, "FIAKER",
			"Changing the reference is currently not well-tested. "
			"Please consider starting a fresh FIAKER window and setting the reference there. "
			"Are you sure you want to continue?",
			QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
		{
			return;
		}
		auto & ui = m_resultUIs[m_referenceID];
		setResultBackground(ui, m_main3DWidget->palette().color(ui.nameActions->backgroundRole()));
		m_showResultVis[m_referenceID]->setText(m_showResultVis[m_referenceID]->text().left(m_showResultVis[m_referenceID]->text().length()-RefMarker.length()));
	}
	addInteraction(QString("Reference set to %1.").arg(resultName(referenceID)));
	auto bounds = m_resultUIs[referenceID].main3DVis->bounds();
	bool setBB = true;
	for (int i = 0; i < 6; ++i)
	{
		if (m_teBoundingBox[i]->text() != "0")
		{
			setBB = false;
			break;
		}
	}
	if (setBB)
	{
		for (int i = 0; i < 3; ++i)
		{
			m_teBoundingBox[i]->setText(QString::number(bounds[i * 2]));
			m_teBoundingBox[i + 3]->setText(QString::number(bounds[i * 2 + 1]));
		}
	}
	m_refDistCompute = new iARefDistCompute(m_data, referenceID);
	if (measures.size() > 0)
	{
		m_refDistCompute->setMeasuresToCompute(measures, optimizationMeasure, bestMeasure);
	}
	connect(m_refDistCompute, &QThread::finished, this, &iAFiAKErController::refDistAvailable);
	m_views[JobView]->show();
	m_jobs->addJob("Computing Reference Similarities", m_refDistCompute->progress(), m_refDistCompute);
	m_refDistCompute->start();
}

namespace
{
	void loadSettings(iASettings const & settings, QMap<QString, QObject*> const & settingsWidgetMap)
	{
		for (QString key : settingsWidgetMap.keys())
		{
			if (settings.contains(key))
			{
				QObject* w = settingsWidgetMap[key];
				if (qobject_cast<QComboBox*>(w))
				{
					int idx = qobject_cast<QComboBox*>(w)->findText(settings.value(key).toString());
					if (idx != -1)
					{
						qobject_cast<QComboBox*>(w)->setCurrentIndex(idx);
					}
				}
				else if (qobject_cast<QCheckBox*>(w))
				{
					qobject_cast<QCheckBox*>(w)->setChecked(settings.value(key).toBool());
				}
				else if (qobject_cast<QSlider*>(w))
				{
					qobject_cast<QSlider*>(w)->setValue(settings.value(key).toInt());
				}
				else if (qobject_cast<QDoubleSpinBox*>(w))
				{
					qobject_cast<QDoubleSpinBox*>(w)->setValue(settings.value(key).toDouble());
				}
				else if (qobject_cast<QSpinBox*>(w))
				{
					qobject_cast<QSpinBox*>(w)->setValue(settings.value(key).toInt());
				}
				else if (qobject_cast<QLineEdit*>(w))
				{
					qobject_cast<QLineEdit*>(w)->setText(settings[key].toString());
				}
				else if (qobject_cast<iAFileChooserWidget*>(w))
				{
					qobject_cast<iAFileChooserWidget*>(w)->setText(settings[key].toString());
				}
				else if (dynamic_cast<iAQLineEditVector*>(w))
				{
					auto & lineEditVector = *dynamic_cast<iAQLineEditVector*>(w);
					QStringList values = settings.value(key).toString().split(",");
					if (values.size() != lineEditVector.size())
					{
						DEBUG_LOG(QString("Invalid value '%1' for key=%2 - should be able to split that into %3 values, but encountered %4")
							.arg(settings.value(key).toString())
							.arg(key)
							.arg(lineEditVector.size())
							.arg(values.size()));
					}
					for (int i = 0; i < lineEditVector.size() && i < values.size(); ++i)
						lineEditVector[i]->setText(values[i]);
				}
				else if (dynamic_cast<iAQCheckBoxVector*>(w))
				{
					auto & checkBoxVector = *dynamic_cast<iAQCheckBoxVector*>(w);
					QStringList values = settings.value(key).toString().split(",");
					// first uncheck all entries:
					for (auto checkbox: checkBoxVector)
						checkbox->setChecked(false);
					QString fullStr = settings.value(key).toString();
					if (fullStr.isEmpty())
						continue;
					// then check those mentioned in settings:
					for (QString v : values)
					{
						bool ok;
						int idx = v.toInt(&ok);
						if (!ok || idx < 0 || idx > checkBoxVector.size())
						{
							DEBUG_LOG(QString("Invalid value '%1' for key=%2; entry %3 is either not convertible to int or outside of valid range 0..%4.")
								.arg(settings.value(key).toString())
								.arg(key)
								.arg(idx)
								.arg(checkBoxVector.size()));
						}
						else
						{
							checkBoxVector[idx]->setChecked(true);
						}
					}
				}
				else
				{
					DEBUG_LOG(QString("Widget type for key=%1 unknown!").arg(key));
				}
			}
			else
			{
				DEBUG_LOG(QString("No value found for key=%1 in settings.").arg(key));
			}
		}
	}

	void saveSettings(QSettings & settings, QMap<QString, QObject*> const & settingsWidgetMap)
	{
		for (QString key : settingsWidgetMap.keys())
		{
			QObject* w = settingsWidgetMap[key];
			if (qobject_cast<QComboBox*>(w))
			{
				settings.setValue(key, qobject_cast<QComboBox*>(w)->currentText());
			}
			else if (qobject_cast<QCheckBox*>(w))
			{
				settings.setValue(key, qobject_cast<QCheckBox*>(w)->isChecked());
			}
			else if (qobject_cast<QSlider*>(w))
			{
				settings.setValue(key, qobject_cast<QSlider*>(w)->value());
			}
			else if (qobject_cast<QDoubleSpinBox*>(w))
			{
				settings.setValue(key, qobject_cast<QDoubleSpinBox*>(w)->value());
			}
			else if (qobject_cast<QSpinBox*>(w))
			{
				settings.setValue(key, qobject_cast<QSpinBox*>(w)->value());
			}
			else if (qobject_cast<QLineEdit*>(w))
			{
				settings.setValue(key, qobject_cast<QLineEdit*>(w)->text());
			}
			else if (qobject_cast<iAFileChooserWidget*>(w))
			{
				settings.setValue(key, qobject_cast<iAFileChooserWidget*>(w)->text());
			}
			else if (dynamic_cast<iAQLineEditVector*>(w))
			{
				QStringList values;
				auto & list = *dynamic_cast<iAQLineEditVector*>(w);
				for (auto edit: list)
					values << edit->text();
				settings.setValue(key, values.join(","));
			}
			else if (dynamic_cast<iAQCheckBoxVector*>(w))
			{
				auto & list = *dynamic_cast<iAQCheckBoxVector*>(w);
				QStringList values;
				for (int i = 0; i < list.size(); ++i)
				{
					if (list[i]->isChecked())
					{
						values << QString::number(i);
					}
				}
				settings.setValue(key, values.join(","));
			}
			else
			{
				DEBUG_LOG(QString("Widget type for key=%1 unknown!").arg(key));
			}
		}
	}
}

bool iAFiAKErController::loadReferenceInternal(iASettings settings)
{
	QString refIDStr = settings.value(ProjectFileReference, "").toString();
	if (refIDStr.isEmpty())
	{
		return false;
	}
	size_t referenceID = NoResult;
	for (size_t resultID = 0; resultID < m_data->result.size(); ++resultID)
	{
		if (QFileInfo(m_data->result[resultID].fileName).completeBaseName() == refIDStr)
		{
			DEBUG_LOG(QString("Result %1, number=%2 will be used as reference!").arg(refIDStr).arg(resultID));
			referenceID = resultID;
			break;
		}
	}
	if (referenceID == NoResult)
	{
		bool ok;
		referenceID = refIDStr.toULongLong(&ok);
		if (!ok || referenceID >= m_data->result.size())
		{
			DEBUG_LOG(QString("Invalid reference specification '%1' in project file! "
				"Expected either a file name (new format) or a result number (old format)").arg(refIDStr));
			return false;
		}
		else
		{
			DEBUG_LOG(QString("Old style project file: result number %1 will be used as reference!").arg(referenceID));
		}
	}
	connect(this, &iAFiAKErController::referenceComputed, [this, settings]
	{   // defer loading the rest of the settings until reference is computed
		loadSettings(settings);
	});
	setReference(referenceID, std::vector<std::pair<int,bool>>(), 0, 0);
	return true;
}

void iAFiAKErController::loadReference(iASettings settings)
{
	if (!loadReferenceInternal(settings))
	{   // if no reference set, load settings directly
		loadSettings(settings);
	}
}

namespace
{
	typedef void (vtkCamera::*SetMethod)(double const[3]);

	void setCameraParameter(iASettings const & settings, QString const & key, vtkCamera* cam, SetMethod method)
	{
		if (!settings.contains(key))
		{
			return;
		}
		double values[3];
		if (stringToArray<double>(settings.value(key).toString(), values, 3, ","))
		{
			(cam->*method)(values);
		}
		else
		{
			DEBUG_LOG(QString("Invalid value %1 for key=%2 couldn't be parsed as double array of size 3!")
				.arg(settings.value(key).toString())
				.arg(key));
		}
	}
}

void iAFiAKErController::loadSettings(iASettings settings)
{
	m_spm->loadSettings(settings);
	::loadSettings(settings, m_settingsWidgetMap);

	auto cam = m_ren->GetActiveCamera();
	setCameraParameter(settings, CameraPosition, cam, &vtkCamera::SetPosition);
	setCameraParameter(settings, CameraViewUp, cam, &vtkCamera::SetViewUp);
	setCameraParameter(settings, CameraFocalPoint, cam, &vtkCamera::SetFocalPoint);

	QByteArray state = settings.value(WindowState).value<QByteArray>();
	m_mdiChild->restoreState(state, 0);
	//loadWindowSettings(settings);
}

void iAFiAKErController::saveSettings(QSettings & settings)
{
	if (m_referenceID != NoResult)
	{
		settings.setValue(ProjectFileReference, QFileInfo(m_data->result[m_referenceID].fileName).completeBaseName());
	}
	m_spm->saveSettings(settings);
	::saveSettings(settings, m_settingsWidgetMap);

	auto cam = m_ren->GetActiveCamera();
	settings.setValue(CameraPosition, arrayToString(cam->GetPosition(), 3, ","));
	settings.setValue(CameraViewUp, arrayToString(cam->GetViewUp(), 3, ","));
	settings.setValue(CameraFocalPoint, arrayToString(cam->GetFocalPoint(), 3, ","));
	settings.setValue(WindowState, m_mdiChild->saveState(0));
	//saveWindowSettings(settings);
}

void iAFiAKErController::refDistAvailable()
{
	if (m_refDistCompute->columnsAdded() == 0)
	{
		delete m_refDistCompute;
		m_refDistCompute = nullptr;
		return;
	}
	size_t startIdx = m_refDistCompute->columnsBefore();
	std::vector<size_t> changedSpmColumns;
	assert(startIdx + m_refDistCompute->columnsAdded() == m_data->spmData->numParams());
	for (size_t col = startIdx; col < startIdx+m_refDistCompute->columnsAdded(); ++col)
	{
		changedSpmColumns.push_back(col);
	}
	m_data->spmData->updateRanges(changedSpmColumns);
	m_referenceID = m_refDistCompute->referenceID();
	m_spnboxReferenceCount->setMaximum(std::min(iARefDistCompute::MaxNumberOfCloseFibers, static_cast<int>(m_data->result[m_referenceID].fiberCount)));
	std::vector<char> v(m_data->spmData->numParams(), false);
	v[0] = v[1] = v[2] = true;
	m_spm->setData(m_data->spmData, v);
	//m_spm->update();
	delete m_refDistCompute;
	m_refDistCompute = nullptr;

	auto & ui = m_resultUIs[m_referenceID];
	QColor refBGColor(m_mainWnd->palette().color(QPalette::AlternateBase));
	setResultBackground(ui, refBGColor);
	m_showResultVis[m_referenceID]->setText(m_showResultVis[m_referenceID]->text() + RefMarker);

	updateRefDistPlots();

	for (size_t spmParamIdx = startIdx; spmParamIdx < m_data->spmData->numParams(); ++spmParamIdx)
	{
		size_t measureIdx = m_data->m_measures.size() - (m_data->spmData->numParams() - spmParamIdx);
		auto diffAvgAction = new QAction(m_data->spmData->parameterName(spmParamIdx), nullptr);
		diffAvgAction->setProperty("colID", static_cast<unsigned long long>(measureIdx+1)); // 0 is Fiber Count
		diffAvgAction->setCheckable(true);
		diffAvgAction->setChecked(false);
		connect(diffAvgAction, &QAction::triggered, this, &iAFiAKErController::stackedColSelect);
		m_stackedBarsHeaders->contextMenu()->addAction(diffAvgAction);
	}
	size_t measureStartIdx = m_data->m_measures.size() - (m_data->spmData->numParams() - startIdx);
	auto measureNames = getAvailableDissimilarityMeasureNames();
	for (int measureIdx = measureStartIdx; measureIdx < m_data->m_measures.size(); ++measureIdx)
	{
		m_settingsView->cmbboxSimilarityMeasure->addItem(measureNames[m_data->m_measures[measureIdx]]);
	}

	QSignalBlocker cblock(m_distributionChoice);
	m_distributionChoice->setCurrentIndex(m_data->spmData->numParams() - 1);
	QSignalBlocker cbColorByBlock(m_colorByDistribution);
	m_colorByDistribution->setChecked(true);
	showMainVis(m_referenceID, Qt::Checked);
	changeDistributionSource(m_data->spmData->numParams() - 1);

	m_views[JobView]->hide();

	emit referenceComputed();
}

void iAFiAKErController::showSpatialOverviewButton()
{
	addInteraction("Showing Spatial Overview.");
	showSpatialOverview();
}

void iAFiAKErController::selectionModeChanged(int mode)
{
	m_style->setSelectionMode(static_cast<iASelectionInteractorStyle::SelectionMode>(mode));
}

void iAFiAKErController::showSpatialOverview()
{
	if (m_referenceID == NoResult)
	{
		return;
	}
	double range[2] = {-1.0, 1.0};
	QSharedPointer<iALookupTable> lut(new iALookupTable());
	*lut = iALUT::Build(range, m_colorByThemeName, 255, SelectionOpacity);
	auto ref3D = m_resultUIs[m_referenceID].main3DVis;
	QSignalBlocker cbBlock(m_showResultVis[m_referenceID]);
	m_showResultVis[m_referenceID]->setChecked(true);
	size_t colID = m_data->result[m_referenceID].table->GetNumberOfColumns()-1;
	ref3D->setLookupTable(lut, colID);
	ref3D->updateColorSelectionRendering();
	setClippingPlanes(ref3D);
	ref3D->show();
	update3D();
	if (!m_cameraInitialized)
	{
		m_ren->ResetCamera();
		m_cameraInitialized = true;
	}
}

void iAFiAKErController::spmLookupTableChanged()
{
	QSharedPointer<iALookupTable> lut = m_spm->lookupTable();
	size_t colorLookupParam = m_spm->colorLookupParam();
	// TODO:
	//     - select distribution in combobox?
	/*
	if (colorLookupParam != m_distributionChoice->currentIndex())
	{
		QSignalBlocker
	}
	*/
	//     - update color theme name if changed in SPM settings
	for (size_t resultID = 0; resultID < m_resultUIs.size(); ++resultID)
	{
		if (m_resultUIs[resultID].main3DVis->visible() && (!matchQualityVisActive() || resultID != m_referenceID))
		{
			m_resultUIs[resultID].main3DVis->setLookupTable(lut, colorLookupParam);
		}
	}
}

void iAFiAKErController::showReferenceToggled()
{
	bool showRef = m_chkboxShowReference->isChecked();
	addInteraction(QString("Show reference fibers toggled to %1.").arg(showRef?"on":"off"));
	changeReferenceDisplay();
}

void iAFiAKErController::showReferenceCountChanged(int count)
{
	addInteraction(QString("Reference fibers count changed to %1.").arg(count));
	changeReferenceDisplay();
}

void iAFiAKErController::showReferenceMeasureChanged(int index)
{
	addInteraction(QString("Selected reference match measure #%1.").arg(index));
	changeReferenceDisplay();
}

void iAFiAKErController::showReferenceLinesToggled()
{
	bool showLines = m_chkboxShowLines->isChecked();
	addInteraction(QString("Show lines to reference fibers toggled to %1.").arg(showLines ? "on" : "off"));
	changeReferenceDisplay();
}

void iAFiAKErController::changeReferenceDisplay()
{
	size_t similarityMeasure = clamp(0, m_data->m_measures.size(), m_settingsView->cmbboxSimilarityMeasure->currentIndex());
	bool showRef = m_chkboxShowReference->isChecked();
	int refCount = std::min(iARefDistCompute::MaxNumberOfCloseFibers, m_spnboxReferenceCount->value());

	if (m_nearestReferenceVis)
	{
		m_nearestReferenceVis->hide();
		m_nearestReferenceVis.clear();
	}

	if (m_refLineActor)
	{
#if VTK_MAJOR_VERSION < 9
		m_main3DWidget->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->RemoveActor(m_refLineActor);
#else
		m_main3DWidget->renderWindow()->GetRenderers()->GetFirstRenderer()->RemoveActor(m_refLineActor);
#endif
	}
	if (!isAnythingSelected() || !showRef)
	{
		update3D();
		return;
	}
	if (m_referenceID == NoResult)
	{
		DEBUG_LOG(QString("You need to set a reference first!"));
		return;
	}
	m_refVisTable = vtkSmartPointer<vtkTable>::New();
	m_refVisTable->Initialize();
	// ID column (int):
	vtkSmartPointer<vtkIntArray> arrID = vtkSmartPointer<vtkIntArray>::New();
	arrID->SetName(m_data->result[m_referenceID].table->GetColumnName(0));
	m_refVisTable->AddColumn(arrID);
	// other columns (float):
	for (int col = 1; col < m_data->result[m_referenceID].table->GetNumberOfColumns() - 1; ++col)
	{
		addColumn(m_refVisTable, 0, m_data->result[m_referenceID].table->GetColumnName(col), 0);
	}

	std::vector<iAFiberSimilarity> referenceIDsToShow;

	//DEBUG_LOG("Showing reference fibers:");
	for (size_t resultID=0; resultID < m_selection.size(); ++resultID)
	{
		if (resultID == m_referenceID || !resultSelected(m_resultUIs, resultID))
		{
			continue;
		}
		for (size_t fiberIdx = 0; fiberIdx < m_selection[resultID].size(); ++fiberIdx)
		{
			size_t fiberID = m_selection[resultID][fiberIdx];
			for (int n=0; n<refCount; ++n)
			{
				referenceIDsToShow.push_back(m_data->result[resultID].refDiffFiber[fiberID].dist[similarityMeasure][n]);
			}
		}
	}
	if (referenceIDsToShow.empty())
	{
		return;
	}

	m_refVisTable->SetNumberOfRows(referenceIDsToShow.size());
	std::map<size_t, std::vector<iAVec3f> > refCurvedFiberInfo;

	auto refTable = m_data->result[m_referenceID].table;
	auto refCurveInfo = m_data->result[m_referenceID].curveInfo;
	for (size_t fiberIdx=0; fiberIdx<referenceIDsToShow.size(); ++fiberIdx)
	{
		size_t refFiberID = referenceIDsToShow[fiberIdx].index;
		double dissimilarity = referenceIDsToShow[fiberIdx].dissimilarity;
		for (int colIdx = 0; colIdx < refTable->GetNumberOfColumns(); ++colIdx)
		{
			m_refVisTable->SetValue(fiberIdx, colIdx, refTable->GetValue(refFiberID, colIdx));
		}
		// set projection error value to dissimilarity...
		m_refVisTable->SetValue(fiberIdx, refTable->GetNumberOfColumns()-2, dissimilarity);

		auto it = refCurveInfo.find(refFiberID);
		if (it != refCurveInfo.end())
		{
			refCurvedFiberInfo.insert(std::make_pair(fiberIdx, it->second));
		}
	}

	m_nearestReferenceVis = QSharedPointer<iA3DCylinderObjectVis>(new iA3DCylinderObjectVis(m_ren, m_refVisTable,
		m_data->result[m_referenceID].mapping, QColor(0,0,0), refCurvedFiberInfo) );
	/*
	QSharedPointer<iALookupTable> lut(new iALookupTable);
	*lut.data() = iALUT::Build(m_data->spmData->paramRange(m_data->spmData->numParams()-iARefDistCompute::EndColumns-iARefDistCompute::SimilarityMeasureCount+similarityMeasure),
		m_colorByThemeName, 256, SelectionOpacity);
	*/
	m_nearestReferenceVis->show();
	// for now, color by reference result color:
	m_nearestReferenceVis->setColor(getResultColor(m_referenceID));
	// ... and set up color coding by it!
	//m_nearestReferenceVis->setLookupTable(lut, refTable->GetNumberOfColumns()-2);
	// TODO: show similarity color map somewhere!!!


	// Lines from Fiber points to reference:
	if (!m_chkboxShowLines->isChecked())
	{
		return;
	}

	auto colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
	colors->SetNumberOfComponents(4);
	colors->SetName("Colors");
	QColor ConnectColor(128, 128, 128);
	unsigned char c[4];
	c[0] = ConnectColor.red();
	c[1] = ConnectColor.green();
	c[2] = ConnectColor.blue();
	c[3] = ConnectColor.alpha();
	size_t colorCount = m_refVisTable->GetNumberOfRows() * 4;
	for (size_t row = 0; row < colorCount; ++row)
	{
		colors->InsertNextTypedTuple(c);
	}
	auto points = vtkSmartPointer<vtkPoints>::New();
	auto linePolyData = vtkSmartPointer<vtkPolyData>::New();
	auto lines = vtkSmartPointer<vtkCellArray>::New();
	size_t curFiber = 0;
	auto & ref = m_data->result[m_referenceID];
	size_t step = static_cast<size_t>(m_optimStepSlider->value());
	for (size_t resultID = 0; resultID < m_selection.size(); ++resultID)
	{
		auto & d = m_data->result[resultID];
		if (resultID == m_referenceID || !resultSelected(m_resultUIs, resultID))
		{
			continue;
		}
		for (size_t fiberIdx = 0; fiberIdx < m_selection[resultID].size(); ++fiberIdx)
		{
			size_t fiberID = m_selection[resultID][fiberIdx];
			for (int n = 0; n < refCount; ++n)
			{
				iAVec3f start1, start2, end1, end2;
				size_t refFiberID = d.refDiffFiber[fiberID].dist[similarityMeasure][n].index;
				for (int i = 0; i < 3; ++i)
				{
					if (d.stepData == iAFiberCharData::SimpleStepData)
					{
						start1[i] = d.stepValues[std::min(step, d.stepValues.size() - 1)][fiberID][i];
					}
					else
					{
						start1[i] = d.table->GetValue(fiberID, d.mapping->value(iACsvConfig::StartX + i)).ToFloat();
					}
					end1[i] = ref.table->GetValue(refFiberID, ref.mapping->value(iACsvConfig::StartX + i)).ToFloat();
				}
				for (int i = 0; i < 3; ++i)
				{
					if (d.stepData == iAFiberCharData::SimpleStepData)
					{
						start2[i] = d.stepValues[std::min(step, d.stepValues.size() - 1)][fiberID][3 + i];
					}
					else
					{
						start2[i] = d.table->GetValue(fiberID, d.mapping->value(iACsvConfig::EndX + i)).ToFloat();
					}
					end2[i] = ref.table->GetValue(refFiberID, ref.mapping->value(iACsvConfig::EndX + i)).ToFloat();
				}
				if ((start1 - start2).length() > (start1 - end2).length() && (end1 - end2).length() > (end1 - start2).length())
				{
					iAVec3f tmp = start1;
					start1 = start2;
					start2 = tmp;
				}
				points->InsertNextPoint(start1.data());
				points->InsertNextPoint(end1.data());
				auto line1 = vtkSmartPointer<vtkLine>::New();
				line1->GetPointIds()->SetId(0, 4 * curFiber); // the index of line start point in pts
				line1->GetPointIds()->SetId(1, 4 * curFiber + 1); // the index of line end point in pts
				lines->InsertNextCell(line1);
				points->InsertNextPoint(start2.data());
				points->InsertNextPoint(end2.data());
				auto line2 = vtkSmartPointer<vtkLine>::New();
				line2->GetPointIds()->SetId(0, 4 * curFiber + 2); // the index of line start point in pts
				line2->GetPointIds()->SetId(1, 4 * curFiber + 3); // the index of line end point in pts
				lines->InsertNextCell(line2);
				++curFiber;
			}
		}
	}
	linePolyData->SetPoints(points);
	linePolyData->SetLines(lines);
	linePolyData->GetPointData()->AddArray(colors);

	auto ids = vtkSmartPointer<vtkIdTypeArray>::New();
	ids->SetName("OriginalIds");
	ids->SetNumberOfTuples(points->GetNumberOfPoints());
	for (vtkIdType id = 0; id < points->GetNumberOfPoints(); ++id)
		ids->SetTuple1(id, id);
	linePolyData->GetPointData()->AddArray(ids);

	auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();

	mapper->SelectColorArray("Colors");
	mapper->SetScalarModeToUsePointFieldData();
	mapper->ScalarVisibilityOn();
	mapper->SetInputData(linePolyData);

	m_refLineActor = vtkSmartPointer<vtkActor>::New();
	m_refLineActor->SetMapper(mapper);
	m_refLineActor->GetProperty()->SetLineWidth(2);
#if VTK_MAJOR_VERSION < 9
	m_main3DWidget->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(m_refLineActor);
#else
	m_main3DWidget->renderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(m_refLineActor);
#endif
	update3D();
}

void iAFiAKErController::playPauseOptimSteps()
{
	QPushButton* btn = qobject_cast<QPushButton*>(sender());
	if (m_playTimer->isActive())
	{
		addInteraction(QString("Stopped optimization step animation."));
		m_playTimer->stop();
		btn->setText("Play");
	}
	else
	{
		addInteraction(QString("Started optimization step animation."));
		m_playTimer->start();
		btn->setText("Pause");
	}
}

void iAFiAKErController::playTimer()
{
	QSignalBlocker block(m_optimStepSlider);
	int newStep = (m_optimStepSlider->value() + 1) % (m_optimStepSlider->maximum() + 1);
	m_optimStepSlider->setValue(newStep);
	setOptimStep(newStep);
}

void iAFiAKErController::playDelayChanged(int newInterval)
{
	addInteraction(QString("Changed optimization step animation delay to %1.").arg(newInterval));
	m_playTimer->setInterval(newInterval);
}

void iAFiAKErController::visualizeCylinderSamplePoints()
{
	size_t resultID, fiberID = NoPlotsIdx;
	for (resultID = 0; resultID < m_selection.size(); ++resultID)
	{
		if (m_selection[resultID].size() > 0)
		{
			fiberID = m_selection[resultID][0];
			break;
		}
	}
	if (fiberID == NoPlotsIdx)
	{
		DEBUG_LOG("No fiber selected!");
		return;
	}
	addInteraction(QString("Visualized cylinder sampling for fiber %1 in %2.").arg(fiberID).arg(resultName(resultID)));
	hideSamplePointsPrivate();

	auto & d = m_data->result[resultID];
	auto const & mapping = *d.mapping.data();
	std::vector<iAVec3f> sampledPoints;
	auto it = d.curveInfo.find(fiberID);
	iAFiberData sampleFiber(d.table, fiberID, mapping, it != d.curveInfo.end() ? it->second : std::vector<iAVec3f>());
	samplePoints(sampleFiber, sampledPoints);
	auto sampleData = vtkSmartPointer<vtkPolyData>::New();
	auto points = vtkSmartPointer<vtkPoints>::New();
	for (size_t s = 0; s < sampledPoints.size(); ++s)
	{
		double pt[3];
		for (int i = 0; i < 3; ++i)
		pt[i] = sampledPoints[s][i];
		points->InsertNextPoint(pt);
	}
	sampleData->SetPoints(points);
	auto vertexFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();
	vertexFilter->SetInputData(sampleData);
	vertexFilter->Update();

	// For color:
	auto polydata = vtkSmartPointer<vtkPolyData>::New();
	polydata->DeepCopy(vertexFilter->GetOutput());
	auto colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
	colors->SetNumberOfComponents(3);
	colors->SetName ("Colors");
	unsigned char blue[3] = {0, 0, 255};
	for (size_t s = 0; s < sampledPoints.size(); ++s)
	{
		colors->InsertNextTypedTuple(blue);
	}
	polydata->GetPointData()->SetScalars(colors);

	auto sampleMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	m_sampleActor = vtkSmartPointer<vtkActor>::New();
	sampleMapper->SetInputData(polydata);
	m_sampleActor->SetMapper(sampleMapper);
	sampleMapper->Update();
	m_sampleActor->GetProperty()->SetPointSize(2);
#if VTK_MAJOR_VERSION < 9
	m_main3DWidget->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(m_sampleActor);
#else
	m_main3DWidget->renderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(m_sampleActor);
#endif
	update3D();
}

void iAFiAKErController::hideSamplePoints()
{
	if (!m_sampleActor)
	{
		return;
	}
	addInteraction("Hide cylinder sampling points.");
	hideSamplePointsPrivate();
	update3D();
	m_sampleActor = nullptr;
}

void iAFiAKErController::hideSamplePointsPrivate()
{
	if (m_sampleActor)
	{
#if VTK_MAJOR_VERSION < 9
		m_main3DWidget->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->RemoveActor(m_sampleActor);
#else
		m_main3DWidget->renderWindow()->GetRenderers()->GetFirstRenderer()->RemoveActor(m_sampleActor);
#endif
	}
}

void iAFiAKErController::optimDataToggled(int state)
{
	size_t chartID = QObject::sender()->property("chartID").toULongLong();
	addInteraction(QString("Toggled visibility of %1 vs. optimization step chart to %2.").arg(diffName(chartID)).arg(state ? "on" : "off"));
	toggleOptimStepChart(chartID, state == Qt::Checked);
}

void iAFiAKErController::selectionFromListActivated(QModelIndex const & index)
{
	auto item = m_selectionListModel->itemFromIndex(index);
	int row = item->row();
	addInteraction(QString("Switched to selection %1.").arg(row));
	m_selection = m_selections[row];
	showSelectionDetail();
	showSelectionIn3DViews();
	showSelectionInPlots();
	showSelectionInSPM();
	changeReferenceDisplay();
}

void iAFiAKErController::showSelectionDetail()
{
	m_selectionDetailModel->clear();
	for (size_t resultID = 0; resultID < m_selection.size(); ++resultID)
	{
		if (m_selection[resultID].size() == 0)
		{
			continue;
		}
		auto resultItem = new QStandardItem(resultName(resultID));
		resultItem->setData(static_cast<unsigned long long>(resultID), Qt::UserRole);
		m_selectionDetailModel->appendRow(resultItem);
		for (size_t selID = 0; selID < m_selection[resultID].size(); ++selID)
		{
			size_t fiberID = m_selection[resultID][selID];
			auto item = new QStandardItem(QString("%1").arg(fiberID+1));
			item->setData(static_cast<unsigned long long>(fiberID), Qt::UserRole);
			resultItem->appendRow(item);
		}
	}
}

void iAFiAKErController::selectionDetailsItemClicked(QModelIndex const & index)
{
	auto item = m_selectionDetailModel->itemFromIndex(index);
	if (!item->hasChildren())
	{   // item text can be changed by users, so use internal data!
		size_t resultID = item->parent()->data(Qt::UserRole).toULongLong();
		size_t fiberID  = item->data(Qt::UserRole).toULongLong();
		addInteraction(QString("Focus on fiber %1 in %2.").arg(fiberID).arg(resultName(resultID)));
		clearSelection();
		m_selection[resultID].push_back(fiberID);
		showSelectionIn3DViews();
		showSelectionInPlots();
		showSelectionInSPM();
		changeReferenceDisplay();
	}
}

QString iAFiAKErController::diffName(size_t chartID) const
{
	size_t spmCol = m_data->m_projectionErrorColumn + chartID;
	return m_data->spmData->parameterName(spmCol);
}

QString iAFiAKErController::resultName(size_t resultID) const
{
	return QFileInfo(m_data->result[resultID].fileName).baseName();
}

/*
void iAFiAKErController::doSaveProject()
{
	// somehow move that part out into the core?
	// { e.g. into iASavableProject ?
	QString fileName = QFileDialog::getSaveFileName(
	QApplication::activeWindow(),
		tr("Select Output File"),
		m_data->folder,
		iAIOProvider::NewProjectFileTypeFilter);
	if (fileName.isEmpty())
	{
		return;
	}
	QSettings projectFile(fileName, QSettings::IniFormat);
	projectFile.setIniCodec("UTF-8");
	projectFile.beginGroup(FIAKERProjectID);
	saveProject(projectFile, fileName);
	projectFile.endGroup();
	projectFile.sync(); // make sure file is written here...
	m_mainWnd->setCurrentFile(fileName); // ...because otherwise it won't get added to recent list here
	addInteraction(QString("Saved as Project '%1'.").arg(fileName));
}
*/

void iAFiAKErController::saveProject(QSettings & projectFile, QString  const & fileName)
{
	projectFile.setValue(ProjectFileFolder, MakeRelative(QFileInfo(fileName).absolutePath(), m_data->folder));
	m_config.save(projectFile, ProjectFileSaveFormatName);
	projectFile.setValue(ProjectFileStepShift, m_data->stepShift);
	projectFile.setValue(ProjectUseStepData, m_useStepData);
	projectFile.setValue(ProjectShowPreviews, m_showPreviews);
	saveSettings(projectFile);
}

void iAFiAKErController::update3D()
{
#if VTK_MAJOR_VERSION < 9
	m_main3DWidget->GetRenderWindow()->Render();
#else
	m_main3DWidget->renderWindow()->Render();
#endif
	m_main3DWidget->update();
}

void iAFiAKErController::setClippingPlanes(QSharedPointer<iA3DColoredPolyObjectVis> vis)
{
	if (m_mdiChild->renderSettings().ShowSlicers)
	{
		auto iaren = m_mdiChild->renderer();
		vtkPlane* planes[3] = { iaren->plane1(), iaren->plane2(), iaren->plane3() };
		vis->setClippingPlanes(planes);
	}
	else
	{
		vis->removeClippingPlanes();
	}
}

void iAFiAKErController::applyRenderSettings()
{
	for (size_t resultID = 0; resultID < m_resultUIs.size(); ++resultID)
	{
		auto mainVis = m_resultUIs[resultID].main3DVis;

		if (m_resultUIs[resultID].vtkWidget)
		{
#if VTK_MAJOR_VERSION < 9
			auto ren = m_resultUIs[resultID].vtkWidget->GetRenderWindow()->GetRenderers()->GetFirstRenderer();
#else
			auto ren = m_resultUIs[resultID].vtkWidget->renderWindow()->GetRenderers()->GetFirstRenderer();
#endif
			ren->SetUseDepthPeeling(m_mdiChild->renderSettings().UseDepthPeeling);
#if (VTK_MAJOR_VERSION >= 8 && defined(VTK_OPENGL2_BACKEND) && QT_VERSION >= QT_VERSION_CHECK(5, 4, 0) )
			ren->SetUseDepthPeelingForVolumes(m_mdiChild->renderSettings().UseDepthPeeling);
#endif
			ren->SetMaximumNumberOfPeels(m_mdiChild->renderSettings().DepthPeels);
			ren->SetUseFXAA(m_mdiChild->renderSettings().UseFXAA);
			QColor bgTop(m_mdiChild->renderSettings().BackgroundTop);
			QColor bgBottom(m_mdiChild->renderSettings().BackgroundBottom);
			ren->SetBackground2(bgTop.redF(), bgTop.greenF(), bgTop.blueF());
			ren->SetBackground(bgBottom.redF(), bgBottom.greenF(), bgBottom.blueF());
		}

		if (mainVis->visible())
		{
			setClippingPlanes(mainVis);
		}
	}
}

void iAFiAKErController::showReferenceInChartToggled()
{
	addInteraction(QString("Toggled showing of reference in distribution charts in result list to %1")
		.arg(m_settingsView->cbShowReferenceDistribution->isChecked()?"on":"off"));
	updateRefDistPlots();
}

void iAFiAKErController::linkPreviewsToggled()
{
	if (!m_showPreviews)
	{
		return;
	}
	bool link = m_settingsView->cbLinkPreviews->isChecked();
	addInteraction(QString("Toggled linking preview and main 3D view to %1")
		.arg(link ? "on" : "off"));
	for (size_t resultID = 0; resultID < m_data->result.size(); ++resultID)
	{
		auto & ui = m_resultUIs[resultID];
#if VTK_MAJOR_VERSION < 9
		auto ren = ui.vtkWidget->GetRenderWindow()->GetRenderers()->GetFirstRenderer();
#else
		auto ren = ui.vtkWidget->renderWindow()->GetRenderers()->GetFirstRenderer();
#endif
		if (link)
		{
			m_renderManager->addToBundle(ren);
		}
		else
		{
			m_renderManager->removeFromBundle(ren);
		}
	}
}

void iAFiAKErController::distributionChartTypeChanged(int idx)
{
	addInteraction(QString("Distribution chart plot type switched to %1.")
		.arg(m_settingsView->cmbboxDistributionPlotType->itemText(idx)));
	changeDistributionSource(m_distributionChoice->currentIndex());
}

void iAFiAKErController::toggleDockWidgetTitleBars()
{
	for (auto w : m_views)
	{
		w->toggleTitleBar();
	}
}

void iAFiAKErController::toggleSettings()
{
	m_views[SettingsView]->setVisible(!m_views[SettingsView]->isVisible());
}
