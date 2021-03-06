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

#include <map>

class iAGraph
{
public:
	typedef long	idType;

	struct Vertex {
		Vertex()
			: id{0}
			, rank{0}
		{ }
		Vertex(int rank, float posX, float posY)
			: id{0}
			, rank{rank}
			, posX{posX}
			, posY{posY}
		{ }

		idType	id;
		int		rank;
		float	posX;
		float	posY;
	};

	struct Edge
	{
		Edge()
			: vertFrom{0}
			, vertTo{0}
		{ }
		Edge(idType vertFrom, idType vertTo)
			: vertFrom{vertFrom}
			, vertTo{vertTo}
		{ }
		// edge has a direction from vertex 1 to vertex 2
		idType vertFrom, vertTo;
	};

	typedef std::map<idType, Vertex> VerticesMap;
	typedef std::map<idType, Edge> EdgesMap;

	iAGraph();
	idType addVertex(Vertex vert);
	idType addEdge(idType v1, idType v2);
	VerticesMap* getVertices();
	EdgesMap* getEdges();

private:
	VerticesMap m_vetices;
	EdgesMap m_edges;
	idType m_curInd;

	idType getNewID();
};

