/*********************************  open_iA 2016 06  ******************************** *
* **********  A tool for scientific visualisation and 3D image processing  ********** *
* *********************************************************************************** *
* Copyright (C) 2016  C. Heinzl, M. Reiter, A. Reh, W. Li, M. Arikan, J. Weissenb�ck, *
*                     Artem & Alexander Amirkhanov, B. Fr�hler                        *
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
* Contact: FH O� Forschungs & Entwicklungs GmbH, Campus Wels, CT-Gruppe,              *
*          Stelzhamerstra�e 23, 4600 Wels / Austria, Email:                           *
* ************************************************************************************/
 
#include "pch.h"
#include "iAVolumeRenderer.h"

#include "iAConsole.h"
#include "iATransferFunction.h"
#include "iAVolumeSettings.h"

#include <vtkImageData.h>
#include <vtkOpenGLRenderer.h>
#include <vtkOutlineFilter.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindow.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>

iAVolumeRenderer::iAVolumeRenderer(
	iATransferFunction * transfer,
	vtkSmartPointer<vtkImageData> imgData)
:
	volProp(vtkSmartPointer<vtkVolumeProperty>::New()),
	volume(vtkSmartPointer<vtkVolume>::New()),
	volMapper(vtkSmartPointer<vtkSmartVolumeMapper>::New()),
	outlineFilter(vtkSmartPointer<vtkOutlineFilter>::New()),
	outlineMapper(vtkSmartPointer<vtkPolyDataMapper>::New()),
	outlineActor(vtkSmartPointer<vtkActor>::New()),
	//renderer(vtkSmartPointer<vtkOpenGLRenderer>::New()),
	//outlineRenderer(vtkSmartPointer<vtkOpenGLRenderer>::New()),
	currentWindow(0),
	currentBoundingBoxWindow(0)
{
	volProp->SetColor(0, transfer->GetColorFunction());
	volProp->SetScalarOpacity(0, transfer->GetOpacityFunction());
	volMapper->SetBlendModeToComposite();
	volMapper->SetInputData(imgData);
	volume->SetMapper(volMapper);
	volume->SetProperty(volProp);
	volume->SetVisibility(true);
	//renderer->AddVolume(volume);

	outlineFilter->SetInputData(imgData);
	outlineMapper->SetInputConnection(outlineFilter->GetOutputPort());
	outlineActor->GetProperty()->SetColor(0, 0, 0);
	outlineActor->PickableOff();
	outlineActor->SetMapper(outlineMapper);
	//outlineRenderer->AddActor(outlineActor);
}

void iAVolumeRenderer::ApplySettings(iAVolumeSettings const & vs)
{
	volProp->SetAmbient(vs.AmbientLighting);
	volProp->SetDiffuse(vs.DiffuseLighting);
	volProp->SetSpecular(vs.SpecularLighting);
	volProp->SetSpecularPower(vs.SpecularPower);
	volProp->SetInterpolationType(vs.LinearInterpolation);
	volProp->SetShade(vs.Shading);
	volMapper->SetRequestedRenderMode(vs.Mode);
#ifdef VTK_OPENGL2_BACKEND
	volMapper->SetSampleDistance(vs.SampleDistance);
	volMapper->InteractiveAdjustSampleDistancesOff();
#endif
}

double * iAVolumeRenderer::GetOrientation()
{
	return volume->GetOrientation();
}

double * iAVolumeRenderer::GetPosition()
{
	return volume->GetPosition();
}

void iAVolumeRenderer::SetOrientation(double* orientation)
{
	volume->SetOrientation(orientation);
	outlineActor->SetOrientation(orientation);
}

void iAVolumeRenderer::SetPosition(double* position)
{
	volume->SetPosition(position);
	outlineActor->SetPosition(position);
}

void iAVolumeRenderer::AddTo(vtkRenderer* w)
{
	if (currentWindow)
	{
		if (currentWindow != w)
		{
			Remove();
		}
		else
		{
			return;
		}
	}
	w->AddVolume(volume);
	
	// experiment adding a separate renderer for each volume:
	//w->AddRenderer(renderer);
	//vtkCamera* cam = w->GetRenderers()->GetFirstRenderer()->GetActiveCamera();
	//renderer->SetActiveCamera(cam);
	//renderer->SetLayer(2);
	//renderer->SetBackground(1, 0.5, 0.5);
	//renderer->InteractiveOn();

	// - produces better looking output on vtk OpenGL2 backend (using the same renderer there
	//     has the effect that the later added volume appears in front of the others)
	// - but introduced the "bug" that only the volume of the last inserted renderer
	//     was affected by actor interaction, for which there seems to be no workaround

	currentWindow = w;
}

void iAVolumeRenderer::Remove()
{
	if (!currentWindow)
	{
		DEBUG_LOG("RemoveFromWindow called on VolumeRenderer which was not attached to a window!\n");
		return;
	}
	//currentWindow->RemoveRenderer(renderer);
	currentWindow->RemoveVolume(volume);
	currentWindow = 0;
}

void iAVolumeRenderer::AddBoundingBoxTo(vtkRenderer* w)
{
	if (currentBoundingBoxWindow)
	{
		if (currentBoundingBoxWindow != w)
		{
			RemoveBoundingBox();
		}
		else
		{
			return;
		}
	}
	//outlineRenderer->SetLayer(3);
	//outlineRenderer->InteractiveOff();
	//vtkCamera* cam = w->GetRenderers()->GetFirstRenderer()->GetActiveCamera();
	//outlineRenderer->SetActiveCamera(cam);
	//w->AddRenderer(outlineRenderer);
	w->AddActor(outlineActor);
	currentBoundingBoxWindow = w;
}


void iAVolumeRenderer::RemoveBoundingBox()
{
	if (!currentBoundingBoxWindow)
		return;
	//currentBoundingBoxWindow->RemoveRenderer(outlineRenderer);
	currentBoundingBoxWindow->RemoveActor(outlineActor);
	currentBoundingBoxWindow = 0;
}

void iAVolumeRenderer::UpdateBoundingBox()
{
	if (!currentBoundingBoxWindow)
		return;
	outlineActor->SetOrientation(volume->GetOrientation());
	outlineActor->SetPosition(volume->GetPosition());
}


vtkSmartPointer<vtkVolume> iAVolumeRenderer::GetVolume()
{
	return volume;
}

void iAVolumeRenderer::Update()
{
	volume->Update();
	volMapper->Update();
}


void iAVolumeRenderer::SetCuttingPlanes(vtkPlane* p1, vtkPlane* p2, vtkPlane* p3)
{
	volMapper->AddClippingPlane(p1);
	volMapper->AddClippingPlane(p2);
	volMapper->AddClippingPlane(p3);
}

void iAVolumeRenderer::RemoveCuttingPlanes()
{
	volMapper->RemoveAllClippingPlanes();
}
