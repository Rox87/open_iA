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
*          Stelzhamerstra�e 23, 4600 Wels / Austria, Email: c.heinzl@fh-wels.at       *
* ************************************************************************************/
#pragma once

#include <QColor>
#include <QString>

#include "open_iA_Core_export.h"

#define DIM 3

const QString organisationName = "FHW";
const QString applicationName = "open_iA";
const QString ProjectFileExtension = ".mod";

enum IOType
{
	UNKNOWN_READER,
	MHD_READER,
	STL_READER,
	RAW_READER,
	PRO_READER,
	PARS_READER,
	VGI_READER,
	TIF_STACK_READER,
	JPG_STACK_READER,
	PNG_STACK_READER,
	BMP_STACK_READER,
	MHD_WRITER,
	STL_WRITER,
	TIF_STACK_WRITER,
	JPG_STACK_WRITER,
	PNG_STACK_WRITER,
	BMP_STACK_WRITER,
	MPEG_WRITER,
	OGV_WRITER,
	AVI_WRITER,
	CSV_READER,
	XML_READER,
	XML_WRITER,
	VOLUME_STACK_READER,
	VOLUME_STACK_MHD_READER,
	VOLUME_STACK_VOLSTACK_READER,
	VOLUME_STACK_VOLSTACK_WRITER,
	DCM_READER,
	DCM_WRITER, 
	NRRD_READER,
	OIF_READER,
	AM_READER,
	AM_WRITER,
	VTI_READER
};

enum FilterID
{
	UNKNOWN_FILTER,
	GRADIENT_ANISOTROPIC_DIFFUSION,
	BINARY_THINNING,
	BINARY_THRESHOLD,
	BAYESIAN_CLASSIFICATION,
	FIBRE_EXTRACTION,
	OTSU_THRESHOLD,
	MAXIMUM_DISTANCE_THRESHOLD,
	CANNY_EDGE_DETECTION,
	DISCRETE_GAUSSIAN,
	DERIVATIVE,
	HIGHER_ORDER_ACCURATE_DERIVATIVE,
	POINTSET_TO_POINTSET_REGISTRATION,
	IMAGE_TO_IMAGE_REGISTRATION,
	THREE_PLANES,
	MANUAL,
	FHW_FUSION,
	GRADIENT_MAGNITUDE,
	RESAMPLER,
	EXTRACT_IMAGE,
	DIFFERENCE_IMAGE,
	DUAL_VIEW_FUSION,
	SMALL_REGION_STATS,
	FHW_CAST_IMAGE,
	HIERARCHICAL_FBP_REBIN,
	BAYESIAN_PROBABILITY,
	DUAL_VIEWING_BAYESIAN_CLASSIFICATION,
	BAYESIAN_PROBABILITY_MANUAL,
	SUBTRACT_IMAGE,
	WATERSHED,
	CURVATURE_ANISOTROPIC_DIFFUSION,
	INVERT_INTENSITY,
	SIGNED_MAURER_DISTANCE_MAP,
	HIERARCHICAL_FBP_RECONSTRUCTION,
	MAE_SOFTWARE_PIPELINE,
	ADAPTIVE_OTSU_THRESHOLD,
	RATS_THRESHOLD,
	DATATYPE_CONVERSION,
	FDK_RECONSTRUCTION,
	EDEC,
	SIMPLE_CONNECTED_COMPONENT_FILTER,
	SCALAR_CONNECTED_COMPONENT_FILTER,
	SIMPLE_RELABEL_COMPONENT_IMAGE_FILTER,
	GENERAL_THRESHOLD,
	DIRECT_FOURIER_RECONSTRUCTION,
	FHWMAEGRADIENTMAGNITUDE,
	VOID_INTERPOLATION,
	THRESHOLD_FUSION,
	ADD_IMAGES_FUSION,
	EDEC_BASIS_FUNCTIONS,
	EDEC_WEIGHTED_TEMPLATE,
	GRADIENT_REGION_GROWING,
	FORWARD_PROJECTION,
	POROSITY_SPECIMEN_GENERATOR,
	INDIVIDUAL_FIBRE_EXTRACTION,
	ARTIFACT_BASED_FUSION,
	POROSITY_MEASUREMENT,
	FCP_AUTOMATIC,
	OTSU_MULTIPLE_THRESHOLD,
	FIBER_VISUALIZATION,
	COMPUTEHESSIANEIGENANALYSIS,
	INDIVIDUAL_FIBRE_VISUALIZATION,
	INDIVIDUAL_PORE_VISUALIZATION,
	COMPUTEPARTICLETEST,
	BILATERAL,
	TEST_FUNCTION_PARALLEL_COORDINATES,
	FCP_SEGMENTATION,
	FIBRE_METADATA_VISUALIZATION,
	MEDIAN,
	DILATION_FILTER,
	EROSION_FILTER,
	BINARY_THINNING_FILTER,
	VESSEL_ENHANCEMENT_FILTER,
	GPU_GRADIENT_ANISOTROPIC_DIFFUSION,
	HISTOGRAM_FILTER,
	MEAN_OBJECT_VISUALIZATION,
	INDIVIDUAL_OBJECT_VISUALIZATION,
	FCP2_SOFTWARE_PIPELINE,
	ADAPTIVE_HISTOGRAM_EQUALIZATION,
	MULTIMODAL_REGISTRATION,
	MULTIMODAL_VISUALIZATION,
	TRANSFORM_FILE_WRITER,
	TRANSFORM_FILE_READER,
	SIMULATION_TO_XCT_REGISTRATION,
	COMPUTE_LAPLACIAN,
	CONVOLUTION_FILTER,
	FFT_CONVOLUTION_FILTER,
	CORRELATION_FILTER,
	FFT_CORRELATION_FILTER,
	KMEANS_FILTER,
	RANDOM_WALKER,
	EXTENDED_RANDOM_WALKER,
	CALCULATE_PORE_CHARS_CSV,
	RESCALE_IMAGE, 
	FFT_NCC_CPP_FILTER,
	FILTER_LABEL_IMAGE, 
	CONVOLUTION, 
	NORMOLIZED_CORRELATION,
	META_TRACTS,
	MASK_IMAGE,
	INTENSITY_WINDOWING,
	SURROUNDING_FILTER,
	MORPH_WATERSHED
};

const int DefaultMagicLensSize = 120;
const int MinimumMagicLensSize = 10;
const int MaximumMagicLensSize = 8192;
const int DefaultHistogramBins = 2048;

// define preset colors
open_iA_Core_API QColor * PredefinedColors();