/****************************************************************************
**
** Copyright (C) 2016 Klaralvdalens Datakonsult AB (KDAB).
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt3D module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qt3dquick3dextrasplugin.h"
#include <Qt3DExtras/qcuboidmesh.h>
#include <Qt3DExtras/qconemesh.h>
#include <Qt3DExtras/qcylindermesh.h>
#include <Qt3DExtras/qdiffusemapmaterial.h>
#include <Qt3DExtras/qdiffusespecularmapmaterial.h>
#include <Qt3DExtras/qforwardrenderer.h>
#include <Qt3DExtras/qgoochmaterial.h>
#include <Qt3DExtras/qnormaldiffusemapalphamaterial.h>
#include <Qt3DExtras/qnormaldiffusemapmaterial.h>
#include <Qt3DExtras/qnormaldiffusespecularmapmaterial.h>
#include <Qt3DExtras/qpervertexcolormaterial.h>
#include <Qt3DExtras/qphongalphamaterial.h>
#include <Qt3DExtras/qphongmaterial.h>
#include <Qt3DExtras/qplanemesh.h>
#include <Qt3DExtras/qspheremesh.h>
#include <Qt3DExtras/qtexturematerial.h>
#include <Qt3DExtras/qtorusmesh.h>
#include <Qt3DExtras/qtorusgeometry.h>
#include <Qt3DExtras/qspheregeometry.h>
#include <Qt3DExtras/qcuboidgeometry.h>
#include <Qt3DExtras/qplanegeometry.h>
#include <Qt3DExtras/qconegeometry.h>
#include <Qt3DExtras/qcylindergeometry.h>
#include <Qt3DExtras/qtext3dgeometry.h>
#include <Qt3DExtras/qtext3dmesh.h>
#include <QtQml/qqml.h>

QT_BEGIN_NAMESPACE

static const struct {
    const char *type;
    int major, minor;
} qmldir [] = {
    // Scene Graph
    { "LevelOfDetailLoader", 2, 2 },
    // Entities
    { "SkyboxEntity", 2, 0 },
    // Camera Controllers
    { "OrbitCameraController", 2, 0 },
    { "FirstPersonCameraController", 2, 0 },
};

void Qt3DQuick3DExtrasPlugin::registerTypes(const char *uri)
{
    // Framegraphs
    qmlRegisterType<Qt3DExtras::QForwardRenderer>(uri, 2, 0, "ForwardRenderer");

    // Materials
    qmlRegisterType<Qt3DExtras::QPhongMaterial>(uri, 2, 0, "PhongMaterial");
    qmlRegisterType<Qt3DExtras::QPhongAlphaMaterial>(uri, 2, 0, "PhongAlphaMaterial");
    qmlRegisterType<Qt3DExtras::QDiffuseMapMaterial>(uri, 2, 0, "DiffuseMapMaterial");
    qmlRegisterType<Qt3DExtras::QDiffuseSpecularMapMaterial>(uri, 2, 0, "DiffuseSpecularMapMaterial");
    qmlRegisterType<Qt3DExtras::QNormalDiffuseMapAlphaMaterial>(uri, 2, 0, "NormalDiffuseMapAlphaMaterial");
    qmlRegisterType<Qt3DExtras::QNormalDiffuseMapMaterial>(uri, 2, 0, "NormalDiffuseMapMaterial");
    qmlRegisterType<Qt3DExtras::QNormalDiffuseSpecularMapMaterial>(uri, 2, 0, "NormalDiffuseSpecularMapMaterial");
    qmlRegisterType<Qt3DExtras::QPerVertexColorMaterial>(uri, 2, 0, "PerVertexColorMaterial");
    qmlRegisterType<Qt3DExtras::QGoochMaterial>(uri, 2, 0, "GoochMaterial");
    qmlRegisterType<Qt3DExtras::QTextureMaterial>(uri, 2, 0, "TextureMaterial");

    // Meshes
    qmlRegisterType<Qt3DExtras::QConeMesh>(uri, 2, 0, "ConeMesh");
    qmlRegisterType<Qt3DExtras::QConeGeometry>(uri, 2, 0, "ConeGeometry");
    qmlRegisterType<Qt3DExtras::QCuboidMesh>(uri, 2, 0, "CuboidMesh");
    qmlRegisterType<Qt3DExtras::QCuboidGeometry>(uri, 2, 0, "CuboidGeometry");
    qmlRegisterType<Qt3DExtras::QCylinderMesh>(uri, 2, 0, "CylinderMesh");
    qmlRegisterType<Qt3DExtras::QCylinderGeometry>(uri, 2, 0, "CylinderGeometry");
    qmlRegisterType<Qt3DExtras::QPlaneMesh>(uri, 2, 0, "PlaneMesh");
    qmlRegisterType<Qt3DExtras::QPlaneGeometry>(uri, 2, 0, "PlaneGeometry");
    qmlRegisterType<Qt3DExtras::QTorusMesh>(uri, 2, 0, "TorusMesh");
    qmlRegisterType<Qt3DExtras::QTorusGeometry>(uri, 2, 0, "TorusGeometry");
    qmlRegisterType<Qt3DExtras::QSphereMesh>(uri, 2, 0, "SphereMesh");
    qmlRegisterType<Qt3DExtras::QSphereGeometry>(uri, 2, 0, "SphereGeometry");

    // 3D Text
    qmlRegisterType<Qt3DExtras::QText3DGeometry>(uri, 2, 2, "Text3DGeometry");
    qmlRegisterType<Qt3DExtras::QText3DMesh>(uri, 2, 2, "Text3DMesh");

    // Register types provided as QML files compiled into the plugin
    for (int i = 0; i < int(sizeof(qmldir) / sizeof(qmldir[0])); i++) {
        auto path = QLatin1String("qrc:/qt-project.org/imports/Qt3D/Extras/defaults/qml/");
        qmlRegisterType(QUrl(path + qmldir[i].type + QLatin1String(".qml")),
                        uri,
                        qmldir[i].major, qmldir[i].minor,
                        qmldir[i].type);
    }
}


QT_END_NAMESPACE
