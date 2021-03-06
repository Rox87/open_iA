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
#include "iAvtkInteractStyleActor.h"
#include "mdichild.h"
#include "iAChannelSlicerData.h"
#include "iAConsole.h"
#include "iASlicerMode.h"
#include "iAVolumeRenderer.h"

#include <vtkCellPicker.h>
#include <vtkCommand.h>
#include <vtkImageActor.h>
#include <vtkImageData.h>
#include <vtkImageReslice.h>
#include <vtkMath.h>
#include <vtkObjectFactory.h>
#include <vtkVolume.h>
#include <vtkTransform.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCamera.h>
#include <vtkProp3D.h>

#include <vtkAlgorithmOutput.h>
#include <vtkPolyDataMapper.h>
#include <vtkCubeSource.h>
#include <vtkLineSource.h>

#include <vtkSphereSource.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkProperty.h>


/*
namespace
{
	//set the coords based on a slicer mode, keep other one fixed
	void updateCoords(double* origin, double const *pos, int mode)
	{
		//DEBUG_LOG(QString("  Pos: %1, %2, %3").arg(pos[0]).arg(pos[1]).arg(pos[2]));
		switch (mode)
		{
		case iASlicerMode::XY:
			origin[0] += pos[0];
			origin[1] += pos[1];
			break;
		case iASlicerMode::XZ:
			origin[0] += pos[0];
			origin[2] += pos[1];
			break;
		case iASlicerMode::YZ:
			origin[1] += pos[0];
			origin[2] += pos[1];
			break;
		case iASlicerMode::SlicerCount:
			origin[0] += pos[0];
			origin[1] += pos[1];
			origin[2] += pos[2];
			break;
		}
	}
}
*/

vtkStandardNewMacro(iAvtkInteractStyleActor);

iAvtkInteractStyleActor::iAvtkInteractStyleActor():
	m_mdiChild(nullptr),
	m_volumeRenderer(nullptr),
	enable3D(false),
	m_rotationEnabled(true)
{

	try {
		m_transform3D = vtkSmartPointer<vtkTransform>::New();

		for (int i = 0; i < 3; i++) {
			m_SliceInteractorTransform[i] = vtkSmartPointer<vtkTransform>::New();
			m_ReslicerTransform[i] = vtkSmartPointer<vtkTransform>::New();
		}
		std::fill(m_slicerChannel, m_slicerChannel + iASlicerMode::SlicerCount, nullptr);
		InteractionPicker->SetTolerance(100.0);

		for (int i = 0; i < 3; i++) {
			m_currentVolRendererPosition[i] = 0.0;
			m_imageRefOrientation[i] = 0.0;
		}
	}
	catch (std::bad_alloc &/*ba*/) {
		DEBUG_LOG("Error in Memory reservation");
	}

}

void iAvtkInteractStyleActor::initializeAndRenderPolyData(uint thickness)
{
	//DEBUG_LOG("init cube");
	if (!m_image || thickness == 0) return;
	try {
		m_CubeSource_X = vtkSmartPointer<vtkCubeSource>::New();
		m_cubeMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
		m_cubeActor = vtkSmartPointer<vtkActor>::New();

		m_SphereSourceCenter = vtkSmartPointer<vtkSphereSource>::New();
		m_SphereMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
		m_SphereActor = vtkSmartPointer<vtkActor>::New();
		m_cubeActor->GetProperty()->SetColor(1, 0, 0); //(R, G, B);

		m_cubeActor->GetProperty()->SetOpacity(1.0); //(R, G, B);

		m_cubeMapper->SetInputConnection(m_CubeSource_X->GetOutputPort());
		m_cubeActor->SetMapper(m_cubeMapper);
		m_cubeActor->SetDragable(0);
		m_cubeXTransform = vtkSmartPointer<vtkTransform>::New();


		m_SphereMapper->SetInputConnection(m_SphereSourceCenter->GetOutputPort());
		m_SphereActor->SetMapper(m_SphereMapper);
		m_SphereActor->GetProperty()->SetColor(1, 1, 1); //(R, G, B);

		m_SphereActor->GetProperty()->SetOpacity(0.7); //(R, G, B);

		const double * bounds = m_image->GetBounds();
		const double *spacing = m_image->GetSpacing();

		double imageCenter[3];
		imageCenter[0] = (bounds[1] + bounds[0]) / 2.0;
		imageCenter[1] = (bounds[3] + bounds[2]) / 2.0;
		imageCenter[2] = (bounds[5] + bounds[4]) / 2.0;
		m_SphereSourceCenter->SetCenter(imageCenter);
		m_SphereSourceCenter->SetRadius(5.0);

		m_CubeSource_X->Update();
		m_CubeSource_X->SetCenter(imageCenter);
		m_CubeSource_X->SetBounds(bounds[0], bounds[1], 0+imageCenter[1], thickness*spacing[1]+imageCenter[1], bounds[4], bounds[5]);
		m_CubeSource_X->Update();

		//this->createReferenceObject(imageCenter, spacing, 2, bounds, transformationMode::x);
		//this->createAndInitLines(bounds, imageCenter);


		if (m_mdiChild && m_volumeRenderer) {

			//m_volumeRenderer->getCurrentRenderer()->AddActor(m_cubeActor);
			//m_volumeRenderer->currentRenderer()->AddActor(m_SphereActor);
			m_volumeRenderer->update();
		}
	}
	catch (std::bad_alloc &/*ba*/) {
		DEBUG_LOG("could not reserve memory for sphereactor");
	}
}

void iAvtkInteractStyleActor::rotateInterActorProp(vtkSmartPointer<vtkTransform> &transform,  double const *center, double angle, vtkProp3D *prop, uint mode)
{
	vtkSmartPointer<vtkMatrix4x4> mat = vtkSmartPointer<vtkMatrix4x4>::New();
	mat = prop->GetUserMatrix();
	if (mat) {
		//DEBUG_LOG("Matrix successfull");
		transform->SetMatrix(mat);
	}
	//transform->Identity();

	transformationMode myMode = static_cast<transformationMode>(mode);
	rotateAroundAxis(transform, center, myMode, angle);
	prop->SetPosition(transform->GetPosition());
	prop->SetOrientation(transform->GetOrientation());
}

void iAvtkInteractStyleActor::translateInterActor(vtkSmartPointer<vtkTransform> &transform, vtkImageActor *actor, double const *position, uint mode)
{
	//transform->PostMultiply();
	vtkSmartPointer<vtkMatrix4x4> mat = vtkSmartPointer<vtkMatrix4x4>::New();
	//mat = actor->GetMatrix();
	//if (mat) {
	//	//DEBUG_LOG("Matrix successfull");
	//	transform->SetMatrix(mat);
	//}

	TranslateActorMovement(actor, mode, transform, position);
}

void iAvtkInteractStyleActor::TranslateActorMovement(vtkImageActor * actor, uint mode, vtkSmartPointer<vtkTransform> & transform, double const * position)
{
	if (!actor)
	{
		return;
	}

	double actorPosition[3];
	actor->GetPosition(actorPosition);

	switch (m_currentSliceMode)
	{
	case iASlicerMode::XY:
		if (mode == iASlicerMode::XZ) {
			transform->Translate(position[0] + actorPosition[0], 0, 0);

		}
		else if (mode == iASlicerMode::YZ) {
			transform->Translate(position[1] + actorPosition[1], 0, 0);
		}
		break;
	case iASlicerMode::XZ:
		if (mode == iASlicerMode::YZ) {
			transform->Translate(0, position[2] + actorPosition[2], 0);

		}
		else if (mode == iASlicerMode::XY) {
			transform->Translate(position[0], 0, 0);
		}
		break;
	case iASlicerMode::YZ:
		if (mode == iASlicerMode::XY) {
			transform->Translate(0, position[1], 0);

		}
		else if (mode == iASlicerMode::XZ) {
			transform->Translate(0, position[2], 0);
		}
		break;
	 default:
		return;
	}

	//transform->Translate(relMovement);
	transform->Update();
	//transform->MultiplyPoint(actorPosition)

	actor->SetUserTransform(transform);
	actor->Update();
}

void iAvtkInteractStyleActor::rotateAroundAxis(vtkSmartPointer<vtkTransform> & transform, double const * center, transformationMode mode/*uint mode*/, double angle)
{
	if (!transform) return;

	transform->PostMultiply();
	transform->Translate(-center[0], -center[1], -center[2]);
	switch (mode)
	{
	case transformationMode::x: transform->RotateX(angle); //DEBUG_LOG("Rotation X");
		break;
	case transformationMode::y: transform->RotateY(angle); //DEBUG_LOG("Rotation Y");
		break;
	case transformationMode::z: transform->RotateZ(angle); //DEBUG_LOG("Rotation Z");
		break;
	}

	transform->Translate(center[0], center[1], center[2]);
	transform->Update();
}

//void iAvtkInteractStyleActor::rotateReslicer(vtkSmartPointer<vtkTransform> &transform, vtkImageReslice *reslicer,  double const *center, uint mode, double angle)
//{
//
//	this->rotateAroundAxis(transform, center, mode, angle);  //transform is ready;
//
//
//	//m_slicerChannel[sliceMode]->reslicer()->SetInputData(m_image);
//	double const * spacing = m_image->GetSpacing();
//	double const * origin = m_image->GetOrigin(); //origin bei null
//
//	//TODO change axes; depending on slicer mode by switch caes
//	int slicerZAxisIdx = mapSliceToGlobalAxis(mode, iAAxisIndex::Z/*iAAxisIndex::Z*/);
//
//	//ist das immer die Z-Achse?
//	double ofs[3] = { 0.0, 0.0, 0.0 };
//	const int sliceNumber = m_mdiChild->sliceNumber(mode);
//	ofs[slicerZAxisIdx] = sliceNumber * spacing[slicerZAxisIdx];
//	//m_slicerChannel[mode]->reslicer()->SetResliceAxesOrigin(origin[0] + ofs[0], origin[1] + ofs[1], origin[2] + ofs[2]);
//
//
//	m_slicerChannel[mode]->reslicer()->SetOutputExtent(m_image->GetExtent());
//	//m_slicerChannel[sliceMode]->reslicer()->SetOutputOrigin(origin[0] + ofs[0], origin[1] + ofs[1], origin[2] + ofs[2]);
//	m_slicerChannel[mode]->reslicer()->SetOutputSpacing(spacing);
//	m_slicerChannel[mode]->reslicer()->Update();
//	/*	m_slicerChannel[sliceMode]->reslicer()->SetOutputDimensionality(2);*/
//	//geht offensichtlich nur über das rotate transform
//
//		//m_slicerChannel[sliceMode]->reslicer()->SetResliceAxes(m_SliceRotateTransform[sliceMode]->GetMatrix());
//	m_slicerChannel[mode]->reslicer()->SetResliceTransform(m_SliceInteractorTransform[mode]);
//	//m_slicerChannel[sliceMode]->reslicer()->UpdateWholeExtent();
//	m_slicerChannel[mode]->reslicer()->SetInterpolationModeToLinear();
//
//	m_slicerChannel[mode]->reslicer()->Update();
//
//
//
//}

void iAvtkInteractStyleActor::createReferenceObject(double /*const */* center, double const *spacing, uint thickness, const double *bounds, transformationMode mode)
{
	if ((!m_mdiChild) && (!m_volumeRenderer)) return;

	try {
		m_RefCubeSource = vtkSmartPointer<vtkCubeSource>::New();
		m_RefCubeMapper = vtkSmartPointer<vtkPolyDataMapper>::New();

		m_RefCubeMapper->SetInputConnection(m_RefCubeSource->GetOutputPort());
		m_RefCubeActor->SetMapper(m_RefCubeMapper);
		m_RefCubeSource->SetCenter(center);
		m_RefCubeActor->GetProperty()->SetColor(0, 0.7, 0); //(R, G, B);

		m_RefCubeActor->GetProperty()->SetOpacity(0.5); //(R, G, B);
		m_RefCubeActor->SetDragable(0);

		//mode 0: x, 1: y, 2:z
		switch (mode)
		{
		case transformationMode::x:
			m_RefCubeSource->SetBounds(center[0], thickness*spacing[0] + center[0],
				bounds[2], bounds[3],
			bounds[4], bounds[5]);
			break;
		case transformationMode::y:
			m_RefCubeSource->SetBounds(bounds[0], bounds[1]/*+300*/,
				center[1], thickness*spacing[1] + center[1], bounds[4]/*+300*/, bounds[5])/*+300)*/;
			break;
		case transformationMode::z:
			m_RefCubeSource->SetBounds(bounds[0], bounds[1]/*+300*/,
				bounds[2], bounds[3], center[2], thickness*spacing[2] + center[0]);
			break;
		}


		m_volumeRenderer->getCurrentRenderer()->AddActor(m_RefCubeActor);
		m_volumeRenderer->update();


	}
	catch (std::bad_alloc &) {
		DEBUG_LOG("could not reserve memory for sphereactor");

	}
}

//void iAvtkInteractStyleActor::rotateXYZ(vtkSmartPointer<vtkTransform> &transform, double const *center, double const *rotationWXYZ)
//{
//	if (!transform) return;
//
//	transform->PostMultiply();
//	transform->Translate(-center[0], -center[1], -center[2]);
//	transform->RotateWXYZ(rotationWXYZ[0], rotationWXYZ[1], rotationWXYZ[2], rotationWXYZ[3]);
//	transform->Translate(center[0], center[1], center[2]);
//	transform->Update();
//}

void iAvtkInteractStyleActor::createAndInitLines(double const *bounds, double const * center)
{
	if ((!m_mdiChild) && (!m_volumeRenderer)) return;
	 try {
		 vtkSmartPointer<vtkLineSource> refLine[3];
		 vtkSmartPointer<vtkActor> refActor[3];
		 vtkSmartPointer<vtkPolyDataMapper> refMapper[3];

		for (int i = 0; i < 3; i++)
		{
			refLine[i] = vtkSmartPointer<vtkLineSource>::New();
			refActor[i] = vtkSmartPointer<vtkActor>::New();
			refMapper[i] = vtkSmartPointer<vtkPolyDataMapper>::New();

			refMapper[i]->SetInputConnection(refLine[i]->GetOutputPort());
			refActor[i]->SetMapper(refMapper[i]);

		}

		this->initLine(refLine[0], refActor[0], center, bounds[0], bounds[1], 0);
		this->initLine(refLine[1], refActor[1], center, bounds[2], bounds[3], 1);
		this->initLine(refLine[2], refActor[2], center, bounds[4], bounds[5], 2);

		for (int i =0; i < 3; i++)
		{
			m_volumeRenderer->getCurrentRenderer()->AddActor(refActor[i]);
		}

		m_volumeRenderer->update();


	}catch (std::bad_alloc &/*ba*/) {
		DEBUG_LOG("Mem error in line creation");
	}
}

void iAvtkInteractStyleActor::initLine(vtkSmartPointer<vtkLineSource> &line, vtkSmartPointer<vtkActor>& lineActor, double const * center, double min, double max, uint sliceMode)
{
	if ((!line) & (!lineActor)) return;
	if (sliceMode > 2) return;
	double dist_half = (min + max) / 2.0;

	double color[3] = { 0, 0, 0 };
	double point1[3] = { center[0],center[1],center[2] };
	double point2[3] = { center[0],center[1],center[2] };

	color[sliceMode] = 1;
	point1[sliceMode] = center[sliceMode]-dist_half;
	point2[sliceMode] = center[sliceMode]+dist_half;

	line->SetPoint1(point1);
	line->SetPoint2(point2);
	lineActor->GetProperty()->SetColor(color);
	lineActor->GetProperty()->SetOpacity(0.82);
	lineActor->GetProperty()->SetLineWidth(4);
	lineActor->SetDragable(0);
	lineActor->SetPickable(0);
}

void iAvtkInteractStyleActor::translatePolydata(vtkSmartPointer<vtkTransform> &polTransform, vtkSmartPointer<vtkActor> &polyActor, double X, double Y, double Z)
{
	m_cubeXTransform->SetMatrix(m_cubeActor->GetMatrix());
	vtkSmartPointer<vtkMatrix4x4> mat = vtkSmartPointer<vtkMatrix4x4>::New();
	mat = polyActor->GetUserMatrix();
	if (mat) {
		//DEBUG_LOG("Matrix successfull");
		polTransform->SetMatrix(mat);
	}

	polTransform->Translate(X, Y, Z);
	polTransform->Update();
	polyActor->SetUserTransform(polTransform);
	m_volumeRenderer->update();
}

void iAvtkInteractStyleActor::rotatePolydata(vtkSmartPointer<vtkTransform> &polTransform, vtkSmartPointer<vtkActor> &polyActor, const double *center, double angle, transformationMode mode)
{
	if (!polTransform) {
		DEBUG_LOG("TRANSFORM IS nullptr");
		return;
	}
	if (!polyActor) {
		DEBUG_LOG("Actor IS nullptr");
		return;
	}

	this->rotateAroundAxis(polTransform, center, mode, angle);

	polTransform->Update();
	polyActor->SetUserTransform(polTransform);
	//polyActor->SetOrientation(polTransform->GetOrientation());
	m_volumeRenderer->update();
}

void iAvtkInteractStyleActor::Rotate()
{ //todo disable translation
  //rotate about center
	if (enable3D)
	{
		//DEBUG_LOG("Rotate 3d");
		//vtkInteractorStyleTrackballActor::Rotate();
		m_rotation3DEnabled = true;
	}
	else
	{
		m_rotationEnabled = true;
	}
}

void iAvtkInteractStyleActor::Spin()
{
	if (enable3D)
	{
		//DEBUG_LOG("spin")
		vtkInteractorStyleTrackballActor::Spin();
	}
}

void iAvtkInteractStyleActor::OnMouseMove()
{
	vtkInteractorStyleTrackballActor::OnMouseMove();

	//mouse move with shift key = translation //else rotation in 2d
	if (this->Interactor->GetShiftKey()) {
		updateInteractors();

	}
	else if (m_rotationEnabled) { // do rotation of the slicer on mouse move
		this->rotate2D();

		m_rotationEnabled = false;
	}
	else if (enable3D && m_rotation3DEnabled) {

		this->rotate3D();
		m_rotation3DEnabled = false;
	}

	if (!enable3D) {
		this->SetCurrentRenderer(nullptr);
	}
}

void iAvtkInteractStyleActor::initialize(vtkImageData *img, iAVolumeRenderer* volRend,
	iAChannelSlicerData *slicerChannel[3], int currentMode, MdiChild *mdiChild)
{
	if (!img) DEBUG_LOG("no valid image!");

	m_image = img;
	m_volumeRenderer = volRend;
	for (int i = 0; i < iASlicerMode::SlicerCount; ++i)
		m_slicerChannel[i] = slicerChannel[i];
	m_currentSliceMode = currentMode;
	enable3D = (m_currentSliceMode == iASlicerMode::SlicerCount);
	if (!mdiChild)
		DEBUG_LOG("MdiChild not set!");
	m_mdiChild = mdiChild;
	m_image->GetSpacing(m_imageSpacing);
	m_RefTransform = vtkSmartPointer<vtkTransform>::New();
	m_RefCubeActor = vtkSmartPointer<vtkActor>::New();

	//initialize pos of currentSlicer
	if (!enable3D) {
		setPreviouSlicesActorPosition(this->m_slicerChannel[m_currentSliceMode]->actorPosition());

	}


	//TODO remove Later
	/*if (enable3D) {
		initializeAndRenderPolyData(5);
	}*/



}

void iAvtkInteractStyleActor::OnLeftButtonDown()
{
	//if(this->Interactor->GetShiftKey())
		vtkInteractorStyleTrackballActor::OnLeftButtonDown();
}

//void iAvtkInteractStyleActor::OnLeftButtonDown()
//{
//
//	int* clickPos = this->GetInteractor()->GetEventPosition();
//
//		// Pick from this location.
//	vtkSmartPointer<vtkPropPicker>  picker =
//			vtkSmartPointer<vtkPropPicker>::New();
//	picker->Pick(clickPos[0], clickPos[1], 0, this);
//
//	double* pos = picker->GetPickPosition();
//	DEBUG_LOG(QString("Pick pos %1 %2 %3").arg(pos[0]).arg(pos[1]).arg(pos[2]));
//
//	//std::cout << "Pick position (world coordinates) is: "
//	//		<< pos[0] << " " << pos[1]
//	//		<< " " << pos[2] << std::endl;
//	auto *testActor = picker->GetActor();
//	if (testActor) {
//		DEBUG_LOG("actor selected");
//	}
//	else DEBUG_LOG("actor not selected");
//
//	vtkInteractorStyleTrackballActor::OnLeftButtonDown();
//	//std::cout << "Picked actor: " << picker->GetActor() << std::endl;
//}

void iAvtkInteractStyleActor::updateInteractors()
{

	// DEBUG_LOG(QString("Move: %1").arg(enable3D ? "3D" : getSlicerModeString(m_currentSliceMode)));

	//coords initialized from the image origin;
	double origin[3];
	m_image->GetOrigin(origin);

	// relative movement of object - we take the position the object was moved to
	// add that to the origin of the image, and reset the position
	//for 3d

	if (enable3D)
	{
		//DEBUG_LOG("starting 3d movement");
		double const * posVolNew = m_volumeRenderer->position();
		if (posVolNew[0] == 0 && posVolNew[1] == 0 && posVolNew[2] == 0)
			return;

		/*end original code*/
		//similar thing like update coords;
		//original coords += positions;

		//old - new
		double relMovement[3] = { 0, 0, 0 };

		relMovement[0] = m_currentVolRendererPosition[0] - posVolNew[0];
		relMovement[1] = m_currentVolRendererPosition[1] - posVolNew[1];
		relMovement[2] = m_currentVolRendererPosition[2] - posVolNew[2];

		if ((relMovement[0] == 0.0) && (relMovement[1] == 0.0) && (relMovement[2] == 0.0))
		{
			//DEBUG_LOG("no movement");
			return;
		}


		//prepare transform for every slicer
		//relative translation of the interactor
		//translate from 3d to 2d

		//for the actors

		//m_SliceInteractorTransform[0]->Translate(relMovement[0], relMovement[1], 0);//relative movement for xy
		//m_SliceInteractorTransform[1]->Translate(relMovement[0], relMovement[2], 0);//rel movement xz
		//m_SliceInteractorTransform[2]->Translate(relMovement[1], relMovement[2], 0); //rel movement yz


		//double trans_xy[3] = { 0/*relMovement[0],*/ ,relMovement[1],0 };
		//double trans_xy[3] = {relMovement[0] ,relMovement[1],relMovement[2]};

	/*	double trans_xz[3] = { relMovement[0], 0,  relMovement[2] };
		double trans_yz[3] = {0, relMovement[1],  relMovement[2] };*/
		//double trans_z[3] = { /*0*/ relMovement[0] ,relMovement[1],/*0*/ relMovement[2] };

		//reslicer plane XZ moving by xy
		//reslicer plane yz moving by yz
		//reslicer plane xy moving by z

		this->TranslateReslicer(m_ReslicerTransform[0], m_slicerChannel[0]->reslicer(),relMovement/*trans_xy*/, m_image->GetSpacing(), /*0,*/ m_image->GetCenter());
		this->TranslateReslicer(m_ReslicerTransform[1], m_slicerChannel[1]->reslicer(),relMovement/*trans_xy*/, m_image->GetSpacing(), /*1,*/ m_image->GetCenter());
		this->TranslateReslicer(m_ReslicerTransform[2], m_slicerChannel[2]->reslicer(),relMovement/*trans_z */, m_image->GetSpacing(), /*2,*/ m_image->GetCenter());

		//store current position of renderer
		this->setPreviousVolActorPosition(posVolNew);

	}
	else //update 2d Slicer
	{
		if (!m_slicerChannel[m_currentSliceMode])
		{
			return;
		}
		/*
		auto render = this->GetCurrentRenderer();
		if (!render)
		{
			DEBUG_LOG("#############################current renderer not initialised");
			return;
		}
		else
		{
			DEBUG_LOG("##########################renderer is active");
		}
		*/
		// This is a translation of current slicer/actor

		//DEBUG_LOG(QString("2D translation %1").arg(m_currentSliceMode));
		double sliceActorPos[3];
		auto tmpslicepos = m_slicerChannel[m_currentSliceMode]->actorPosition();
		std::copy(tmpslicepos, tmpslicepos + 3, sliceActorPos);
		double spacing[3];
		m_image->GetSpacing(spacing);

		// //mapping slicer from yz -> x, y	xy -> x, y		xz->x, y //0 -> xy, //1 -> xz,	//2 -> yz
		//prepare the coords - relative movement in
		double absMovementXYZ[3] = { 0,0,0 };
		prepareMoventCoords(absMovementXYZ, sliceActorPos, false);

		if ((absMovementXYZ[0] == 0.0) && (absMovementXYZ[1] == 0.0) && (absMovementXYZ[2] == 0.0)) {
			return;
		}

		update3DUpdateReslicer(absMovementXYZ, sliceActorPos);

	}

	//update reslicer + actor
	for (int i = 0; i < iASlicerMode::SlicerCount; ++i)
		if (i != m_currentSliceMode && m_slicerChannel[i])
			m_slicerChannel[i]->updateReslicer();

	m_volumeRenderer->update();
	emit actorsUpdated();
}

void iAvtkInteractStyleActor::update3DUpdateReslicer(double const * movementXYZ, double const * sliceActorPos)
{
	if (!movementXYZ) return;

	double const *volRendPos = m_volumeRenderer->volume()->GetPosition();
	//DEBUG_LOG(QString("VolActorbefore position %1 %2 %3").arg(volRendPos[0]).arg(volRendPos[1]).arg(volRendPos[2]));
	//DEBUG_LOG(QString("movement 3d %1 %2 %3").arg(movementXYZ[0]).arg(movementXYZ[1]).arg(movementXYZ[2]));

	//movement in xyz
	//relative position + volactor in 3d
	//double newPosition_3dAbs[3];
	double slicerMoventXYZ[3];
	for (int i = 0; i < 3; i++)
	{
		//newPosition_3dAbs[i] = movementXYZ[i] + m_currentVolRendererPosition[i];
		slicerMoventXYZ[i] = -movementXYZ[i];
	}

	double OutPos[3] = { 0, 0,0 };
	m_transform3D->Translate(movementXYZ);
	m_transform3D->Update();
	m_transform3D->TransformPoint(volRendPos, OutPos);
	m_volumeRenderer->volume()->SetPosition(m_transform3D->GetPosition()); //not via setting transform!
	//pass coordinates in 3d
	double volRendPosafter[3] = { 0, 0, 0 };
	m_volumeRenderer->volume()->GetPosition(volRendPosafter);
	//double translation[3] = { volRendPosafter[0],volRendPosafter[1],volRendPosafter[2] };
	this->setPreviouSlicesActorPosition(sliceActorPos); //setting to original
	this->setPreviousVolActorPosition(volRendPosafter);
	m_slicerChannel[m_currentSliceMode]->setActorPosition(0, 0, 0);

	for (int i = 0; i < 3; i++) {
		this->TranslateReslicer(m_ReslicerTransform[i], m_slicerChannel[i]->reslicer(), slicerMoventXYZ, m_image->GetSpacing(), m_image->GetCenter());
	}
}

void iAvtkInteractStyleActor::reset()
{
	throw std::invalid_argument("not yet implemented");
	/*m_volumeRenderer->volume()->SetPosition(0, 0, 0);
	m_volumeRenderer->volume()->SetOrientation(0, 0, 0);*/

	//here volume should reset, orientation of reslicer should be reset
	//this->setPreviousVolActorPosition()
	//this->setPreviouSlicesActorPosition()
}

void iAvtkInteractStyleActor::TranslateActor(double const * movement, uint mode)
{
	//DEBUG_LOG(QString("movement %1 %2 %3 ").arg(movement[0]).arg(movement[1]).arg(movement[2]));
	//mode YZ ->translate xz and xz
	//mode xy ->translate xz, yz
	//mode xz ->tranlate xy, yz

	switch (mode)
		{

		case iASlicerMode::XY:

			//DEBUG_LOG("Translation yz Translation XZ");

			performTranslationTransform(m_SliceInteractorTransform[0], m_slicerChannel[0]->imageActor(), movement, 0);
			performTranslationTransform(m_SliceInteractorTransform[1], m_slicerChannel[1]->imageActor(), movement, 1);
			break;

		case iASlicerMode::XZ:
			//DEBUG_LOG("Translation xy, Translation xy");
			performTranslationTransform(m_SliceInteractorTransform[0], m_slicerChannel[0]->imageActor(), movement, 0);
			performTranslationTransform(m_SliceInteractorTransform[2], m_slicerChannel[2]->imageActor(), movement, 2);
			break;

		case iASlicerMode::YZ:
			//DEBUG_LOG("Translation xy, Translation xz");
			performTranslationTransform(m_SliceInteractorTransform[1], m_slicerChannel[1]->imageActor(), movement, 1);
			performTranslationTransform(m_SliceInteractorTransform[2], m_slicerChannel[2]->imageActor(), movement, 2);
			break;

		default:
			DEBUG_LOG("no interaction");
			break;
		}

}

void iAvtkInteractStyleActor::performTranslationTransform(vtkSmartPointer<vtkTransform> &transform, vtkImageActor *actor, double const *relMovement, uint mode)
{

	//DEBUG_LOG(QString("translation vector %1 %2 %3").arg(relMovement[0]).arg(relMovement[1]).arg(relMovement[2]));

	if (!relMovement) return;
	/*vtkSmartPointer<vtkMatrix4x4> mat = vtkSmartPointer<vtkMatrix4x4>::New();
	mat = actor->GetUserMatrix();
	if (mat)
		transform->SetMatrix(mat);*/

	switch (mode) {
	case iASlicerMode::XY:
		//DEBUG_LOG("translate xy");
		transform->Translate(relMovement[0], relMovement[1], 0);

		break;
	case iASlicerMode::XZ:
		//DEBUG_LOG("translate xz");
		transform->Translate(relMovement[0], relMovement[2], 0);
		break;

	case iASlicerMode::YZ:
		//DEBUG_LOG("translate yz");
		transform->Translate(relMovement[1], relMovement[2], 0);
		break;

	default:
		break;

	}

	transform->Update();
	//transform[0]->Translate(relMovement[0], relMovement[1], 0);
	////rel movement xz
	//transform[1]->Translate(relMovement[1], relMovement[2], 0);
	////rel movement yz
	//transform[2]->Translate(relMovement[1], relMovement[2], 0);
	double posOut[3] = { 0,0, 0, };

	transform->GetPosition(posOut);//m_currentSliceActorPosition, posOut);
	actor->SetPosition(posOut);
	actor->Update();
}



void iAvtkInteractStyleActor::prepareMoventCoords(double * movement, double const * sliceActorPos, bool relativeMovement)
{  //position = new - old
	//actor position is in 2d

	double oldPos[2] = { 0, 0 };
	if (relativeMovement)
	{
		oldPos[0] = m_currentSliceActorPosition[0];
		oldPos[1] = m_currentSliceActorPosition[1];
	}

	switch (m_currentSliceMode) {
	case iASlicerMode::XY:
		movement[0] = /*oldPos[0] - newPos[0]*/sliceActorPos[0] - oldPos[0];
		movement[1] = /*oldPos[1] - newPos[1]*/ sliceActorPos[1] - oldPos[1];
		break;
	case iASlicerMode::XZ:
		movement[0] = /*oldPos[0] - newPos[0];*/sliceActorPos[0] - oldPos[0];
		movement[2] = /*oldPos[1] - newPos[1];*/sliceActorPos[1] - oldPos[1];
		break;
	case iASlicerMode::YZ:
		movement[1] = /*oldPos[0] - newPos[0];*/sliceActorPos[0] - oldPos[0];
		movement[2] = /*oldPos[1] - newPos[1];*/sliceActorPos[1] - oldPos[1];
		break;
	}

	//DEBUG_LOG(QString("OldActorPos %1 %2").arg(oldPos[0]).arg(oldPos[1]));
	//DEBUG_LOG(QString("sliceActorPos %1 %2").arg(sliceActorPos[0]).arg(sliceActorPos[1]).arg(sliceActorPos[2]));
	//DEBUG_LOG(QString("movemnt %1 %2 %3").arg(movement[0]).arg(movement[1]).arg(movement[2]));
}

void iAvtkInteractStyleActor::rotate2D()
{
	//2d rotation  work in this way:
	/*
	*1 perform perform rotation to interactor
	*2  update 3d volume -> yes
	*3 update slicer
	*/


	//DEBUG_LOG("Rotation");
	if (this->CurrentRenderer == nullptr || this->InteractionProp == nullptr)
	{
		return;
	}

	vtkRenderWindowInteractor *rwi = this->Interactor;
	if (!m_image) { DEBUG_LOG(QString("Error on rotation %1").arg(m_currentSliceMode)); return; }

	// Get the axis to rotate around = vector from eye to origin
	double const *imageBounds = this->m_image->GetBounds();

	double imageCenter[3];//image center
	imageCenter[0] = (imageBounds[1] + imageBounds[0]) / 2.0;
	imageCenter[1] = (imageBounds[3] + imageBounds[2]) / 2.0;
	imageCenter[2] = (imageBounds[5] + imageBounds[4]) / 2.0;

	//center of interaction prop of current slicer
	double *sliceProbCenter = this->InteractionProp->GetCenter();
	double disp_obj_center[3];
	double relativeAngle = 0.0;

	//2d, rotate Z;
	computeDisplayRotationAngle(sliceProbCenter, disp_obj_center, rwi, relativeAngle);

	//rotation of current slicer
	//DEBUG_LOG(QString("Current slice mode %1 ").arg(m_currentSliceMode));

	transformationMode rotationDir;
	switch (m_currentSliceMode)
	{
	default: DEBUG_LOG("Invalid slicer mode"); // intentional fall-through - C++ 17: [[fallthrough]]
	case iASlicerMode::YZ: rotationDir = transformationMode::x; break;
	case iASlicerMode::XZ: rotationDir = transformationMode::y; relativeAngle *= -1.0;  break;
	case iASlicerMode::XY: rotationDir = transformationMode::z; break;
	}

	this->ReslicerRotate(m_ReslicerTransform[m_currentSliceMode], m_slicerChannel[m_currentSliceMode]->reslicer(),
		rotationDir, imageCenter, -relativeAngle, m_image->GetSpacing());

	for (int i = 0; i < 3; i++) {
		if (i == m_currentSliceMode) continue;
		this->ReslicerRotate(m_ReslicerTransform[i], m_slicerChannel[i]->reslicer(),
			rotationDir, imageCenter, relativeAngle, m_image->GetSpacing());
	}

	//this->rotateInterActorProp(m_SliceInteractorTransform[m_currentSliceMode], sliceProbCenter, relativeAngle, this->InteractionProp,2);

	//double const * spacing = m_image->GetSpacing();
	//double const *volImageCenter = m_volumeRenderer->volume()->GetCenter();
	//double const *orientationBefore = m_volumeRenderer->volume()->GetOrientation();
	//DEBUG_LOG(QString("orientation before %1 %2 %3").arg(orientationBefore[0]).arg(orientationBefore[1]).arg(orientationBefore[2]));

	//rotate around axis based on the spacing needed

	//double const *volObjPos = m_volumeRenderer->volume()->GetPosition();
	//DEBUG_LOG(QString("volPosition %1 %2 %3").arg(volObjPos[0]).arg(volObjPos[1]).arg(volObjPos[2]));
	//DEBUG_LOG(QString("volCenter %1 %2 %3").arg(volImageCenter[0]).arg(volImageCenter[1]).arg(volImageCenter[2]));
	//DEBUG_LOG(QString("imageCenter %1 %2 %3").arg(imageCenter[0]).arg(imageCenter[1]).arg(imageCenter[2]));

	//rotate in actor in 3D
	this->rotateInterActorProp(m_transform3D, m_volumeRenderer->volume()->GetCenter(), relativeAngle, m_volumeRenderer->volume(), m_currentSliceMode);

	this->setPreviousVolActorPosition(m_volumeRenderer->volume()->GetPosition());
	this->setPreviouSlicesActorPosition(m_slicerChannel[m_currentSliceMode]->actorPosition());

	//double const *orientationAfter = m_volumeRenderer->volume()->GetOrientation();

	for (int i = 0; i < iASlicerMode::SlicerCount; ++i)
		if (i != m_currentSliceMode && m_slicerChannel[i])
			m_slicerChannel[i]->updateReslicer();

	m_volumeRenderer->update();
	emit actorsUpdated();
}


void iAvtkInteractStyleActor::rotate3D()
{
	//starting from 3d,
	//then rotate reslicer in xyz - each direction
	if (this->CurrentRenderer == nullptr || this->InteractionProp == nullptr)
	{
		return;
	}

	//DEBUG_LOG("rotate 3d");
	this->setRefOrientation(m_volumeRenderer->volume()->GetOrientation());
	vtkRenderWindowInteractor *rwi = this->Interactor;

	vtkCamera *cam = this->CurrentRenderer->GetActiveCamera();

	// First get the origin of the assembly
	double *obj_center = this->InteractionProp->GetCenter();

	// GetLength gets the length of the diagonal of the bounding box
	double boundRadius = this->InteractionProp->GetLength() * 0.5;
	//below copyied from superclass
	// Get the view up and view right vectors
	double view_up[3], view_look[3], view_right[3];

	cam->OrthogonalizeViewUp();
	cam->ComputeViewPlaneNormal();
	cam->GetViewUp(view_up);
	vtkMath::Normalize(view_up);
	cam->GetViewPlaneNormal(view_look);
	vtkMath::Cross(view_up, view_look, view_right);
	vtkMath::Normalize(view_right);

	// Get the furtherest point from object position+origin
	double outsidept[3];

	outsidept[0] = obj_center[0] + view_right[0] * boundRadius;
	outsidept[1] = obj_center[1] + view_right[1] * boundRadius;
	outsidept[2] = obj_center[2] + view_right[2] * boundRadius;
	// Convert them to display coord
	double disp_obj_center[3];

	this->ComputeWorldToDisplay(obj_center[0], obj_center[1], obj_center[2],
		disp_obj_center);

	this->ComputeWorldToDisplay(outsidept[0], outsidept[1], outsidept[2],
		outsidept);

	double radius = sqrt(vtkMath::Distance2BetweenPoints(disp_obj_center,
		outsidept));
	double nxf = (rwi->GetEventPosition()[0] - disp_obj_center[0]) / radius;

	double nyf = (rwi->GetEventPosition()[1] - disp_obj_center[1]) / radius;

	double oxf = (rwi->GetLastEventPosition()[0] - disp_obj_center[0]) / radius;

	double oyf = (rwi->GetLastEventPosition()[1] - disp_obj_center[1]) / radius;

	if (((nxf * nxf + nyf * nyf) <= 1.0) &&
		((oxf * oxf + oyf * oyf) <= 1.0))
	{
		double newXAngle = vtkMath::DegreesFromRadians(asin(nxf));
		double newYAngle = vtkMath::DegreesFromRadians(asin(nyf));
		double oldXAngle = vtkMath::DegreesFromRadians(asin(oxf));
		double oldYAngle = vtkMath::DegreesFromRadians(asin(oyf));

		double scale[3];
		scale[0] = scale[1] = scale[2] = 1.0;

		double **rotate = new double*[2];

		rotate[0] = new double[4];
		rotate[1] = new double[4];

		rotate[0][0] = newXAngle - oldXAngle;
		rotate[0][1] = view_up[0];
		rotate[0][2] = view_up[1];
		rotate[0][3] = view_up[2];

		rotate[1][0] = oldYAngle - newYAngle;
		rotate[1][1] = view_right[0];
		rotate[1][2] = view_right[1];
		rotate[1][3] = view_right[2];

		//rotate 3d volume
		this->Prop3DTransform(this->InteractionProp,
			obj_center,
			2,
			rotate,
			scale);

		double const *orientationAfter = m_volumeRenderer->volume()->GetOrientation();

		//translate in XYZ
		double relRotation[3] = { 0, 0, 0 };
		for (int i = 0; i < 3; i++)
		{
			relRotation[i] = m_imageRefOrientation[i] - orientationAfter[i];
		}

		//xz //rotate z then take z-coordinate of relative Rotati9n
		/*this->ReslicerRotate(this->getResliceTransform(1), this->getReslicer(1),
			transformationMode::z, m_image->GetCenter(), relRotation[2], m_imageSpacing);*/
		//this->ReslicerRotate(this->getResliceTransform(0), this->getReslicer(0),
		//	transformationMode::z, m_image->GetCenter(), relRotation[2], m_imageSpacing);
		//this->ReslicerRotate(this->getResliceTransform(2), this->getReslicer(2), transformationMode::y, m_image->GetCenter(), relRotation[1], m_imageSpacing);

		//rotate resclicer for xyz - coordinates


		//posibly image center from bounds?
		//does the image center change
		for (uint i = 0; i < 3; i++)
		{
			this->rotateReslicerXYZ(this->getResliceTransform(i), this->getReslicer(i),
				relRotation, 2u, m_image->GetCenter(), m_imageSpacing);
		}

		double const *posVol = m_volumeRenderer->volume()->GetPosition();
		this->setPreviousVolActorPosition(posVol);

		delete[] rotate[0];
		delete[] rotate[1];
		delete[] rotate;

		rwi->Render();

		for (int i = 0; i < iASlicerMode::SlicerCount; ++i)
			if (i != m_currentSliceMode && m_slicerChannel[i])
				m_slicerChannel[i]->updateReslicer();

		m_volumeRenderer->update();
		emit actorsUpdated();
	}
}

void iAvtkInteractStyleActor::computeDisplayRotationAngle(double * sliceProbCenter, double * disp_obj_center, vtkRenderWindowInteractor * rwi, double &relativeAngle)
{
	/*drag an angle,
	  calculates angle from coords, objectCenter comes from interaction prop
	*/

	this->ComputeWorldToDisplay(sliceProbCenter[0], sliceProbCenter[1], sliceProbCenter[2],
		disp_obj_center);

	double newAngle =
		vtkMath::DegreesFromRadians(atan2(rwi->GetEventPosition()[1] - disp_obj_center[1],
			rwi->GetEventPosition()[0] - disp_obj_center[0]));

	double oldAngle =
		vtkMath::DegreesFromRadians(atan2(rwi->GetLastEventPosition()[1] - disp_obj_center[1],
			rwi->GetLastEventPosition()[0] - disp_obj_center[0]));

	relativeAngle = newAngle - oldAngle;
}

void iAvtkInteractStyleActor::TranslateReslicer(vtkSmartPointer<vtkTransform> &transform, vtkImageReslice *reslice,
												double const *position, double *spacing, /*int sliceMode*/ double const * /*imageCenter*/)
{
	if ((!reslice) && (!transform))
		return;

	//DEBUG_LOG("Translate reslicer");
	//vtkSmartPointer<vtkMatrix4x4> mat = m_slicerChannel[sliceMode]->reslicer()->GetResliceAxes();
	//if (mat) {
	//	DEBUG_LOG("Setting input matrix");
	//	transform->SetMatrix(mat);
	//}

	//double value = 0;
	//double const *origin = m_image->GetOrigin();

	//int slicerZAxisIdx = mapSliceToGlobalAxis(sliceMode, iAAxisIndex::X/*iAAxisIndex::Z*/);
	//double ofs[3] = { 0.0, 0.0, 0.0 };
	/*const int sliceNumber = m_mdiChild->sliceNumber(sliceMode);
	ofs[slicerZAxisIdx] = sliceNumber * spacing[slicerZAxisIdx];*/

	reslice->SetOutputDimensionality(2);
	transform->Translate(position[0], position[1], position[2]);
	reslice->SetResliceTransform(transform);
	reslice->SetInputData(m_image);
	reslice->SetOutputExtent(m_image->GetExtent());
	reslice->AutoCropOutputOff();


	//reslice->SetOutputOrigin(origin[0] + ofs[0], origin[1] + ofs[1], origin[2] + ofs[2]);
	//m_slicerChannel[sliceMode]->reslicer()->SetOutputOrigin(mageCenter);
	//reslice->SetOutputOrigin(mageCenter);
	/*reslice->SetResliceAxes(mat); */
	reslice->SetOutputSpacing(spacing);
	reslice->SetOutputOrigin(m_image->GetOrigin());
	//reslice->SetResliceAxesOrigin(origin[0] + ofs[0], origin[1] + ofs[1], origin[2] + ofs[2]);
	reslice->SetInterpolationModeToCubic();
	reslice->Update();

}


//void iAvtkInteractStyleActor::TransformReslicerExperimental(double const * obj_center, double rotationAngle, double const *spacing, int sliceMode)
//{
//
//	if (!m_image) { DEBUG_LOG(QString("image is null, slicemode %1").arg(sliceMode)); return;  }
//
//	m_SliceInteractorTransform[sliceMode]->PostMultiply();
//	m_SliceInteractorTransform[sliceMode]->Translate(-obj_center[0],-obj_center[1],/**spacing[1]**spacing[2]*/-obj_center[2]);
//	m_SliceInteractorTransform[sliceMode]->RotateZ(rotationAngle);
//	m_SliceInteractorTransform[sliceMode]->Translate(obj_center[0], obj_center[1],/*0*//**spacing[1]*//*,0*//**spacing[2]*/ obj_center[2]);
//	m_SliceInteractorTransform[sliceMode]->Inverse();
//	const int sliceNumber = m_mdiChild->sliceNumber(sliceMode);  /*m_slicerChannel[m_currentSliceMode]->reslicer()->GetNumber*/
//	//const int sliceMode = m_currentSliceMode;
//	double ofs[3] = { 0.0, 0.0, 0.0 };
//	updateReslicerRotationTransformation2d(sliceMode, ofs, sliceNumber);
//
//	m_volumeRenderer->update();
//
//}

void iAvtkInteractStyleActor::ReslicerRotate(vtkSmartPointer<vtkTransform> &transform, vtkImageReslice *reslicer, transformationMode mode, double const * center, double angle, double const *spacing)
{
	if (!reslicer || !transform || angle == 0.0)
	{
		return;
	}

	//DEBUG_LOG("Rotate Axis");

		//rotate axis in degree
	this->rotateAroundAxis(transform, center, mode, angle);
	prepareReslicer(reslicer, transform, spacing);

}

void iAvtkInteractStyleActor::prepareReslicer(vtkImageReslice * reslicer, vtkSmartPointer<vtkTransform> transform, double const * spacing)
{
	reslicer->SetResliceTransform(transform);
	reslicer->SetInputData(m_image);
	reslicer->SetOutputExtent(m_image->GetExtent());
	reslicer->AutoCropOutputOff();

	/*reslice->SetResliceAxes(mat); */
	reslicer->SetOutputSpacing(spacing);
	reslicer->SetOutputOrigin(m_image->GetOrigin());
	//reslice->SetResliceAxesOrigin(origin[0] + ofs[0], origin[1] + ofs[1], origin[2] + ofs[2]);

	reslicer->SetInterpolationModeToCubic();
	reslicer->Update();
}

void iAvtkInteractStyleActor::rotateReslicerXYZ( vtkSmartPointer<vtkTransform> transform, vtkImageReslice *reslicer, double const *rotXYZ, uint rotationMode, double const * center, double const *spacing)
{
	if (!transform)
	{
		return;
	}

	transform->PostMultiply();
	transform->Translate(-center[0], -center[1], -center[2]);
	switch (rotationMode)
	{
	case 0:
		transform->RotateX(rotXYZ[0]);
		break;
	case 1:
		transform->RotateX(rotXYZ[0]);
		transform->RotateY(rotXYZ[1]);
		break;
	case 2:
		transform->RotateX(rotXYZ[0]);
		transform->RotateY(rotXYZ[1]);
		transform->RotateZ(rotXYZ[2]);

	default:
		break;
	}

	transform->Translate(center[0], center[1], center[2]);
	this->prepareReslicer(reslicer, transform, spacing);
}

//void iAvtkInteractStyleActor::updateReslicerRotationTransformation2d(const int sliceMode, double * ofs, const int sliceNumber)
//{
//
//
//		m_slicerChannel[sliceMode]->reslicer()->SetInputData(m_image);
//		double const * spacing = m_image->GetSpacing();
//		double const * origin = m_image->GetOrigin(); //origin bei null
//		int slicerZAxisIdx = mapSliceToGlobalAxis(sliceMode, iAAxisIndex::Z/*iAAxisIndex::Z*/);
//
//		//ist das immer die Z-Achse?
//		ofs[slicerZAxisIdx] = sliceNumber * spacing[slicerZAxisIdx];
//		m_slicerChannel[sliceMode]->reslicer()->SetResliceAxesOrigin(origin[0] + ofs[0], origin[1] + ofs[1], origin[2] + ofs[2]);
//
//
//
//		m_slicerChannel[sliceMode]->reslicer()->SetOutputExtent(m_image->GetExtent());
//		//m_slicerChannel[sliceMode]->reslicer()->SetOutputOrigin(origin[0] + ofs[0], origin[1] + ofs[1], origin[2] + ofs[2]);
//		m_slicerChannel[sliceMode]->reslicer()->SetOutputSpacing(m_image->GetSpacing());
//		m_slicerChannel[sliceMode]->reslicer()->Update();
//	/*	m_slicerChannel[sliceMode]->reslicer()->SetOutputDimensionality(2);*/
//	//geht offensichtlich nur über das rotate transform
//
//		//m_slicerChannel[sliceMode]->reslicer()->SetResliceAxes(m_SliceRotateTransform[sliceMode]->GetMatrix());
//		m_slicerChannel[sliceMode]->reslicer()->SetResliceTransform(m_SliceInteractorTransform[sliceMode]);
//		//m_slicerChannel[sliceMode]->reslicer()->UpdateWholeExtent();
//		m_slicerChannel[sliceMode]->reslicer()->SetInterpolationModeToLinear();
//
//		m_slicerChannel[sliceMode]->reslicer()->Update();
//		//m_image = /*dynamic_cast<vtkImageData* >(*/m_slicerChannel[/*0*//*m_currentSliceMode*/sliceMode]->reslicer()->GetInformationInput()/*)*/;
//
//}
