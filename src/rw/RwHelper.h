#pragma once

extern bool bDebugRenderGroups;
extern bool gPS2alphaTest;
extern bool gBackfaceCulling;

void OpenCharsetSafe();
void CreateDebugFont();
void DestroyDebugFont();
void ObrsPrintfString(const char *str, short x, short y);
void FlushObrsPrintfs();
void DefinedState(void);
void SetCullMode(uint32 mode);
RwFrame *GetFirstChild(RwFrame *frame);
RwObject *GetFirstObject(RwFrame *frame);
RpAtomic *GetFirstAtomic(RpClump *clump);
RwTexture *GetFirstTexture(RwTexDictionary *txd);

bool IsClumpSkinned(RpClump *clump);
RpHAnimHierarchy *GetAnimHierarchyFromSkinClump(RpClump *clump);	// get from atomic
RpHAnimHierarchy *GetAnimHierarchyFromClump(RpClump *clump);	// get from frame
void SkinGetBonePositionsToTable(RpClump *clump, RwV3d *boneTable);
RpHAnimAnimation *HAnimAnimationCreateForHierarchy(RpHAnimHierarchy *hier);
RpAtomic *AtomicRemoveAnimFromSkinCB(RpAtomic *atomic, void *data);
void RenderSkeleton(RpHAnimHierarchy *hier);

RwBool Im2DRenderQuad(RwReal x1, RwReal y1, RwReal x2, RwReal y2, RwReal z, RwReal recipCamZ, RwReal uvOffset);
RpClump *RpClumpGetBoundingSphere(RpClump *clump, RwSphere *sphere, bool useLTM);

RwTexDictionary *RwTexDictionaryGtaStreamRead(RwStream *stream);
RwTexDictionary *RwTexDictionaryGtaStreamRead1(RwStream *stream);
RwTexDictionary *RwTexDictionaryGtaStreamRead2(RwStream *stream, RwTexDictionary *texDict);
void ReadVideoCardCapsFile(uint32&, uint32&, uint32&, uint32&);
bool CheckVideoCardCaps(void);
void WriteVideoCardCapsFile(void);
bool CanVideoCardDoDXT(void);
void ConvertingTexturesScreen(uint32, uint32, const char*);
void DealWithTxdWriteError(uint32, uint32, const char*);
bool CreateTxdImageForVideoCard();

bool RpClumpGtaStreamRead1(RwStream *stream);
RpClump *RpClumpGtaStreamRead2(RwStream *stream);
void RpClumpGtaCancelStream(void);

void CameraSize(RwCamera *camera,
                RwRect *rect,
                RwReal viewWindow,
                RwReal aspectRatio);
void CameraDestroy(RwCamera *camera);
RwCamera *CameraCreate(RwInt32 width,
                       RwInt32 height,
                       RwBool zBuffer);

					   

RpAtomic *ConvertPlatformAtomic(RpAtomic *atomic, void *data);

#if defined(FIX_BUGS) && defined (GTA_PC)
void SetAlphaTest(RwUInt32 alpharef);
void RestoreAlphaTest();
#else
#define SetAlphaTest(a) (0)
#define RestoreAlphaTest() (0)
#endif