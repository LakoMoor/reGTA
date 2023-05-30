#include "common.h"

#include "AnimBlendAssociation.h"
#include "AnimBlendNode.h"

void 
CAnimBlendNode::Init(void)
{
	frameA = -1;
	frameB = -1;
	remainingTime = 0.0f;
	sequence = nil;
	association = nil;
}

bool
CAnimBlendNode::Update(CVector &trans, CQuaternion &rot, float weight)
{
	bool looped = false;

	trans = CVector(0.0f, 0.0f, 0.0f);
	rot = CQuaternion(0.0f, 0.0f, 0.0f, 0.0f);

	if(association->IsRunning()){
		remainingTime -= association->timeStep;
		if(remainingTime <= 0.0f)
			looped = NextKeyFrame();
	}

	float blend = association->GetBlendAmount(weight);
	if(blend > 0.0f){
		KeyFrameTrans *kfA = (KeyFrameTrans*)sequence->GetKeyFrame(frameA);
		KeyFrameTrans *kfB = (KeyFrameTrans*)sequence->GetKeyFrame(frameB);
		float t = kfA->deltaTime == 0.0f ? 0.0f : (kfA->deltaTime - remainingTime)/kfA->deltaTime;
		if(sequence->type & CAnimBlendSequence::KF_TRANS){
			trans = kfB->translation + t*(kfA->translation - kfB->translation);
			trans *= blend;
		}
		if(sequence->type & CAnimBlendSequence::KF_ROT){
			rot.Slerp(kfB->rotation, kfA->rotation, theta, invSin, t);
			rot *= blend;
		}
	}

	return looped;
}

bool
CAnimBlendNode::UpdateCompressed(CVector &trans, CQuaternion &rot, float weight)
{
	bool looped = false;

	trans = CVector(0.0f, 0.0f, 0.0f);
	rot = CQuaternion(0.0f, 0.0f, 0.0f, 0.0f);

	if(association->IsRunning()){
		remainingTime -= association->timeStep;
		if(remainingTime <= 0.0f)
			looped = NextKeyFrameCompressed();
	}

	float blend = association->GetBlendAmount(weight);
	if(blend > 0.0f){
		KeyFrameTransCompressed *kfA = (KeyFrameTransCompressed*)sequence->GetKeyFrameCompressed(frameA);
		KeyFrameTransCompressed *kfB = (KeyFrameTransCompressed*)sequence->GetKeyFrameCompressed(frameB);
		float t = kfA->deltaTime == 0 ? 0.0f : (kfA->GetDeltaTime() - remainingTime)/kfA->GetDeltaTime();
		if(sequence->type & CAnimBlendSequence::KF_TRANS){
			CVector transA, transB;
			kfA->GetTranslation(&transA);
			kfB->GetTranslation(&transB);
			trans = transB + t*(transA - transB);
			trans *= blend;
		}
		if(sequence->type & CAnimBlendSequence::KF_ROT){
			CQuaternion rotA, rotB;
			kfA->GetRotation(&rotA);
			kfB->GetRotation(&rotB);
			rot.Slerp(rotB, rotA, theta, invSin, t);
			rot *= blend;
		}
	}

	return looped;
}

bool
CAnimBlendNode::NextKeyFrame(void)
{
	bool looped;

	if(sequence->numFrames <= 1)
		return false;

	looped = false;
	frameB = frameA;

	// Advance as long as we have to
	while(remainingTime <= 0.0f){
		frameA++;

		if(frameA >= sequence->numFrames){
			// reached end of animation
			if(!association->IsRepeating()){
				frameA--;
				remainingTime = 0.0f;
				return false;
			}
			looped = true;
			frameA = 0;
		}

		remainingTime += sequence->GetKeyFrame(frameA)->deltaTime;
	}

	frameB = frameA - 1;
	if(frameB < 0)
		frameB += sequence->numFrames;

	CalcDeltas();
	return looped;
}

bool
CAnimBlendNode::NextKeyFrameCompressed(void)
{
	bool looped;

	if(sequence->numFrames <= 1)
		return false;

	looped = false;
	frameB = frameA;

	// Advance as long as we have to
	while(remainingTime <= 0.0f){
		frameA++;

		if(frameA >= sequence->numFrames){
			// reached end of animation
			if(!association->IsRepeating()){
				frameA--;
				remainingTime = 0.0f;
				return false;
			}
			looped = true;
			frameA = 0;
		}

		remainingTime += sequence->GetKeyFrameCompressed(frameA)->GetDeltaTime();
	}

	frameB = frameA - 1;
	if(frameB < 0)
		frameB += sequence->numFrames;

	CalcDeltasCompressed();
	return looped;
}

// Set animation to time t
bool
CAnimBlendNode::FindKeyFrame(float t)
{
	if(sequence->numFrames < 1)
		return false;

	frameA = 0;
	frameB = frameA;

	if(sequence->numFrames == 1){
		remainingTime = 0.0f;
	}else{
		// advance until t is between frameB and frameA
		while (t > sequence->GetKeyFrame(++frameA)->deltaTime) {
			t -= sequence->GetKeyFrame(frameA)->deltaTime;
			if (frameA + 1 >= sequence->numFrames) {
				// reached end of animation
				if (!association->IsRepeating()) {
					CalcDeltas();
					remainingTime = 0.0f;
					return false;
				}
				frameA = 0;
			}
			frameB = frameA;
		}

		remainingTime = sequence->GetKeyFrame(frameA)->deltaTime - t;
	}

	CalcDeltas();
	return true;
}

bool
CAnimBlendNode::SetupKeyFrameCompressed(void)
{
	if(sequence->numFrames < 1)
		return false;

	frameA = 1;
	frameB = 0;

	if(sequence->numFrames == 1){
		frameA = 0;
		remainingTime = 0.0f;
	}else
		remainingTime = sequence->GetKeyFrameCompressed(frameA)->GetDeltaTime();

	CalcDeltasCompressed();
	return true;
}

void
CAnimBlendNode::CalcDeltas(void)
{
	if((sequence->type & CAnimBlendSequence::KF_ROT) == 0)
		return;
	KeyFrame *kfA = sequence->GetKeyFrame(frameA);
	KeyFrame *kfB = sequence->GetKeyFrame(frameB);
	float cos = DotProduct(kfA->rotation, kfB->rotation);
	if(cos > 1.0f)
		cos = 1.0f;
	theta = Acos(cos);
	invSin = theta == 0.0f ?  0.0f : 1.0f/Sin(theta);
}

void
CAnimBlendNode::CalcDeltasCompressed(void)
{
	if((sequence->type & CAnimBlendSequence::KF_ROT) == 0)
		return;
	KeyFrameCompressed *kfA = sequence->GetKeyFrameCompressed(frameA);
	KeyFrameCompressed *kfB = sequence->GetKeyFrameCompressed(frameB);
	CQuaternion rotA, rotB;
	kfA->GetRotation(&rotA);
	kfB->GetRotation(&rotB);
	float cos = DotProduct(rotA, rotB);
	if(cos < 0.0f){
		rotB *= -1.0f;
		kfB->SetRotation(rotB);
	}
	cos = DotProduct(rotA, rotB);
	if(cos > 1.0f)
		cos = 1.0f;
	theta = Acos(cos);
	invSin = theta == 0.0f ?  0.0f : 1.0f/Sin(theta);
}

void
CAnimBlendNode::GetCurrentTranslation(CVector &trans, float weight)
{
	trans = CVector(0.0f, 0.0f, 0.0f);

	float blend = association->GetBlendAmount(weight);
	if(blend > 0.0f){
		KeyFrameTrans *kfA = (KeyFrameTrans*)sequence->GetKeyFrame(frameA);
		KeyFrameTrans *kfB = (KeyFrameTrans*)sequence->GetKeyFrame(frameB);
		float t = kfA->deltaTime == 0.0f ? 0.0f : (kfA->deltaTime - remainingTime)/kfA->deltaTime;
		if(sequence->type & CAnimBlendSequence::KF_TRANS){
			trans = kfB->translation + t*(kfA->translation - kfB->translation);
			trans *= blend;
		}
	}
}

void
CAnimBlendNode::GetCurrentTranslationCompressed(CVector &trans, float weight)
{
	trans = CVector(0.0f, 0.0f, 0.0f);

	float blend = association->GetBlendAmount(weight);
	if(blend > 0.0f){
		KeyFrameTransCompressed *kfA = (KeyFrameTransCompressed*)sequence->GetKeyFrameCompressed(frameA);
		KeyFrameTransCompressed *kfB = (KeyFrameTransCompressed*)sequence->GetKeyFrameCompressed(frameB);
		float t = kfA->deltaTime == 0 ? 0.0f : (kfA->GetDeltaTime() - remainingTime)/kfA->GetDeltaTime();
		if(sequence->type & CAnimBlendSequence::KF_TRANS){
			CVector transA, transB;
			kfA->GetTranslation(&transA);
			kfB->GetTranslation(&transB);
			trans = transB + t*(transA - transB);
			trans *= blend;
		}
	}
}

void
CAnimBlendNode::GetEndTranslation(CVector &trans, float weight)
{
	trans = CVector(0.0f, 0.0f, 0.0f);

	float blend = association->GetBlendAmount(weight);
	if(blend > 0.0f){
		KeyFrameTrans *kf = (KeyFrameTrans*)sequence->GetKeyFrame(sequence->numFrames-1);
		if(sequence->type & CAnimBlendSequence::KF_TRANS)
			trans = kf->translation * blend;
	}
}

void
CAnimBlendNode::GetEndTranslationCompressed(CVector &trans, float weight)
{
	trans = CVector(0.0f, 0.0f, 0.0f);

	float blend = association->GetBlendAmount(weight);
	if(blend > 0.0f){
		KeyFrameTransCompressed *kf = (KeyFrameTransCompressed*)sequence->GetKeyFrameCompressed(sequence->numFrames-1);
		if(sequence->type & CAnimBlendSequence::KF_TRANS){
			CVector pos;
			kf->GetTranslation(&pos);
			trans = pos * blend;
		}
	}
}
