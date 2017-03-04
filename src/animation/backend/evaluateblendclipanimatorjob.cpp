/****************************************************************************
**
** Copyright (C) 2017 Klaralvdalens Datakonsult AB (KDAB).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt3D module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "evaluateblendclipanimatorjob_p.h"
#include <Qt3DAnimation/private/handler_p.h>
#include <Qt3DAnimation/private/managers_p.h>
#include <Qt3DAnimation/private/animationlogging_p.h>
#include <Qt3DAnimation/private/animationutils_p.h>
#include <Qt3DAnimation/private/lerpclipblend_p.h>
#include <Qt3DAnimation/private/clipblendnodevisitor_p.h>
#include <Qt3DAnimation/private/job_common_p.h>

QT_BEGIN_NAMESPACE

namespace Qt3DAnimation {
namespace Animation {

EvaluateBlendClipAnimatorJob::EvaluateBlendClipAnimatorJob()
    : Qt3DCore::QAspectJob()
    , m_currentLoop(std::numeric_limits<int>::max())
    , m_isFinalFrame(true)
{
    SET_JOB_RUN_STAT_TYPE(this, JobTypes::EvaluateBlendClipAnimator, 0);
}

namespace {

ClipResults blendValuesBasedOnMappings(ClipBlendNode *node,
                                       const ClipResults &channelResults1,
                                       const ClipResults &channelResults2,
                                       const QVector<BlendingMappingData> &blendingMappingData)
{
    ClipResults blendedValues;
    blendedValues.reserve(blendingMappingData.size());

    // Build a combined vector of blended value
    for (const BlendingMappingData &mapping : blendingMappingData) {

        switch (mapping.blendAction) {
        case BlendingMappingData::ClipBlending: {
            Q_ASSERT(mapping.channelIndicesClip1.size() == mapping.channelIndicesClip2.size());
            for (int i = 0, m = mapping.channelIndicesClip1.size(); i < m; ++i) {
                const float value1 = channelResults1.at(mapping.channelIndicesClip1[i]);
                const float value2 = channelResults2.at(mapping.channelIndicesClip2[i]);
                const float blendedValue = node->blend(value1, value2);
                blendedValues.push_back(blendedValue);
            }
            break;
        }
        case BlendingMappingData::NoBlending: {
            const bool useClip1 = !mapping.channelIndicesClip1.empty();
            const QVector<int> channelIndices = useClip1 ? mapping.channelIndicesClip1 : mapping.channelIndicesClip2;
            const QVector<float> values = useClip1 ? channelResults1 : channelResults2;
            for (int i = 0, m = channelIndices.size(); i < m; ++i) {
                const float value = values.at(channelIndices[i]);
                blendedValues.push_back(value);
            }
            break;
        }
        default:
            Q_UNREACHABLE();
            break;
        }
    }
    return blendedValues;
}

QVector<MappingData> fromBlendingMappingData(const QVector<BlendingMappingData> &blendingMappingData)
{
    const int blendingMappingDataSize = blendingMappingData.size();
    QVector<MappingData> mappingData(blendingMappingDataSize);
    for (int i = 0; i < blendingMappingDataSize; ++i) {
        mappingData[i] = blendingMappingData[i];
    }
    return mappingData;
}

} // anonymous

void EvaluateBlendClipAnimatorJob::blendClips(ClipBlendNode *node,
                                              const BlendedClipAnimator::BlendNodeData &nodeData,
                                              const AnimatorEvaluationData &animatorEvaluationData)
{
    AnimationClipLoader *clip1 = m_handler->animationClipLoaderManager()->lookupResource(nodeData.left);
    AnimationClipLoader *clip2 = m_handler->animationClipLoaderManager()->lookupResource(nodeData.right);
    Q_ASSERT(clip1 && clip2);

    // Prepare for evaluation (convert global time to local time ....)
    const ClipEvaluationData preEvaluationDataForClip1 = evaluationDataForClip(clip1, animatorEvaluationData);
    const ClipEvaluationData preEvaluationDataForClip2 = evaluationDataForClip(clip2, animatorEvaluationData);

    // Evaluate the fcurves for both clip
    const ClipResults channelResultsClip1 = evaluateClipAtLocalTime(clip1, preEvaluationDataForClip1.localTime);
    const ClipResults channelResultsClip2 = evaluateClipAtLocalTime(clip2, preEvaluationDataForClip2.localTime);

    // Update loops and running of the animator
    m_currentLoop = std::min(m_currentLoop, std::min(preEvaluationDataForClip1.currentLoop, preEvaluationDataForClip2.currentLoop));
    // isFinalFrame remains true only if all the clips have reached their final frame
    m_isFinalFrame &= (preEvaluationDataForClip1.isFinalFrame && preEvaluationDataForClip2.isFinalFrame);

    const QVector<BlendingMappingData> blendingMappingData = nodeData.mappingData;
    // Perform blending between the two clips
    const QVector<float> blendedValues = blendValuesBasedOnMappings(node, channelResultsClip1,
                                                                    channelResultsClip2, blendingMappingData);

    m_clipBlendResultsTable.insert(node, blendedValues);
}

void EvaluateBlendClipAnimatorJob::blendNodes(ClipBlendNode *node, const BlendedClipAnimator::BlendNodeData &nodeData)
{
    ClipBlendNode *node1 = m_handler->clipBlendNodeManager()->lookupNode(nodeData.left);
    ClipBlendNode *node2 = m_handler->clipBlendNodeManager()->lookupNode(nodeData.right);
    Q_ASSERT(node1 && node2);

    // Retrieve results for the childNodes
    const ClipResults channelResultsNode1 = m_clipBlendResultsTable.take(node1);
    const ClipResults channelResultsNode2 = m_clipBlendResultsTable.take(node2);

    // Build a combined vector of blended value
    const QVector<BlendingMappingData> blendingMappingData = nodeData.mappingData;
    // Perform blending between the two nodes
    const QVector<float> blendedValues = blendValuesBasedOnMappings(node, channelResultsNode1,
                                                                    channelResultsNode2, blendingMappingData);

    m_clipBlendResultsTable.insert(node, blendedValues);
}

void EvaluateBlendClipAnimatorJob::run()
{
    const qint64 globalTime = m_handler->simulationTime();

    BlendedClipAnimator *blendedClipAnimator = m_handler->blendedClipAnimatorManager()->data(m_blendClipAnimatorHandle);
    Q_ASSERT(blendedClipAnimator);

    const AnimatorEvaluationData animatorEvaluationData = evaluationDataForAnimator(blendedClipAnimator, globalTime);
    const QHash<Qt3DCore::QNodeId, BlendedClipAnimator::BlendNodeData> blendindNodeTable = blendedClipAnimator->blendTreeTable();

    // Reset globals
    m_currentLoop = std::numeric_limits<int>::max();
    m_isFinalFrame = true;

    // Perform blending of the tree (Post-order traversal)
    ClipBlendNodeVisitor visitor(m_handler->clipBlendNodeManager());
    visitor.traverse(blendedClipAnimator->blendTreeRootId(), [&, this] (ClipBlendNode *blendNode) {
        // Retrieve BlendingData for the not
        const BlendedClipAnimator::BlendNodeData &nodeData = blendindNodeTable.value(blendNode->peerId());

        switch (nodeData.type) {
        case BlendedClipAnimator::BlendNodeData::BlendNodeType:
            // Blend two ClipBlendNode
            blendNodes(blendNode, nodeData);
            break;
        case BlendedClipAnimator::BlendNodeData::ClipType: // Leaf
            // Blend two clips
            blendClips(blendNode, nodeData, animatorEvaluationData);
            break;
        default:
            Q_UNREACHABLE();
            break;
        }
    });


    // Set current loop and running if need
    if (m_isFinalFrame)
        blendedClipAnimator->setRunning(false);
    blendedClipAnimator->setCurrentLoop(m_currentLoop);

    // Prepare property changes using the root node which should now be filled with correct values
    ClipBlendNode *rootBlendNode = m_handler->clipBlendNodeManager()->lookupNode(blendedClipAnimator->blendTreeRootId());
    Q_ASSERT(m_clipBlendResultsTable.size() == 1 && m_clipBlendResultsTable.contains(rootBlendNode));

    const ClipResults blendedValues = m_clipBlendResultsTable.take(rootBlendNode);
    const BlendedClipAnimator::BlendNodeData &rootNodeData = blendindNodeTable.value(rootBlendNode->peerId());
    const QVector<MappingData> mappingData = fromBlendingMappingData(rootNodeData.mappingData);

    // Prepare property changes (if finalFrame it also prepares the change for the running property for the frontend)
    const QVector<Qt3DCore::QSceneChangePtr> changes = preparePropertyChanges(blendedClipAnimator->peerId(),
                                                                                              mappingData,
                                                                                              blendedValues,
                                                                                              m_isFinalFrame);
    // Send the property changes
    blendedClipAnimator->sendPropertyChanges(changes);
}

} // Animation
} // Qt3DAnimation

QT_END_NAMESPACE
