//
// MPF Platform
// Resource Manager
// 作者：SunnyCase
// 创建时间：2016-07-21
//
#pragma once
#include "../../inc/common.h"
#include "ResourceContainer.h"
#include "Geometry.h"
#include "Brush.h"
#include "Pen.h"
#include "Camera.h"
#include "../../inc/WeakReferenceBase.h"
#include "RenderCommandBuffer.h"
#include "Platform/PlatformProvider.h"
#include <DirectXMath.h>

DEFINE_NS_PLATFORM
#include "../MPF.Platform_i.h"

class FontManager;

#define DECL_RESCONTAINERAWARE(T) \
std::shared_ptr<ResourceContainer<T>> _container##T; \
std::vector<UINT_PTR> _added##T;				 \
std::vector<UINT_PTR> _updated##T;					 

#define DECL_RESOURCEMGR_IMPL1(T)		  \
T& Get##T##(UINT_PTR handle);			  \
const T& Get##T##(UINT_PTR handle) const;		 

#define DECL_RESOURCEMGR_TRC_GETTER(T)		  \
virtual ITransformedResourceContainer<T>& Get##T##TRC() noexcept = 0

#define DEFINE_RESOURCEMGR_IMPL1(T)															  \
T& Get##T##(UINT_PTR handle) { return _container##T##->FindResource(handle); }				  \
const T& Get##T##(UINT_PTR handle) const { return _container##T##->FindResource(handle); }

class ResourceManagerBase : public WeakReferenceBase<ResourceManagerBase, WRL::RuntimeClassFlags<WRL::ClassicCom>, IResourceManager>
{
public:
	ResourceManagerBase();

	// 通过 RuntimeClass 继承
	STDMETHODIMP CreateRenderCommandBuffer(IRenderCommandBuffer ** buffer) override;
	STDMETHODIMP CreateFontFaceFromMemory(INT_PTR buffer, UINT64 size, UINT faceIndex, IFontFace **fontFace) override;
	STDMETHODIMP CreateResource(ResourceType resType, IResource ** res) override;
	STDMETHODIMP UpdateLineGeometry(IResource * res, LineGeometryData * data) override;
	STDMETHODIMP UpdateRectangleGeometry(IResource * res, RectangleGeometryData * data) override;
	HRESULT UpdatePathGeometry(IResource * res, std::vector<PathGeometrySegments::Segment>&& segments) noexcept;
	STDMETHODIMP UpdatePathGeometry(IResource * res, byte* data, UINT32 length) override;
	STDMETHODIMP UpdateSolidColorBrush(IResource * res, ColorF * color) override;
	STDMETHODIMP UpdatePen(IResource * res, float thickness, IResource* brush) override;
	STDMETHODIMP UpdateCamera(IResource * res, float* viewMatrix, float* projectionMatrix) override;
	STDMETHODIMP UpdateBoxGeometry3D(IResource * res, BoxGeometry3DData * data) override;

	void RetireResource(IResource * res);

	DEFINE_RESOURCEMGR_IMPL1(LineGeometry);
	DEFINE_RESOURCEMGR_IMPL1(RectangleGeometry);
	DEFINE_RESOURCEMGR_IMPL1(PathGeometry);
	DECL_RESOURCEMGR_IMPL1(Brush);
	DEFINE_RESOURCEMGR_IMPL1(Pen);
	DEFINE_RESOURCEMGR_IMPL1(Camera);
	DEFINE_RESOURCEMGR_IMPL1(BoxGeometry3D);

	void Update();
	virtual std::shared_ptr<IDrawCallList> CreateDrawCallList(RenderCommandBuffer* rcb) = 0;
	void AddDependentDrawCallList(std::weak_ptr<IDrawCallList>&& dcl, IResource* res);

	virtual void BeginResetDevice() {}
	virtual void EndResetDevice() {}
protected:
	DECL_RESOURCEMGR_TRC_GETTER(LineGeometry);
	DECL_RESOURCEMGR_TRC_GETTER(RectangleGeometry);
	DECL_RESOURCEMGR_TRC_GETTER(PathGeometry);
	DECL_RESOURCEMGR_TRC_GETTER(BoxGeometry3D);
	virtual void UpdateOverride() {};
private:
	DECL_RESCONTAINERAWARE(LineGeometry);
	DECL_RESCONTAINERAWARE(RectangleGeometry);
	DECL_RESCONTAINERAWARE(PathGeometry);
	DECL_RESCONTAINERAWARE(SolidColorBrush);
	DECL_RESCONTAINERAWARE(Pen);
	DECL_RESCONTAINERAWARE(Camera);
	DECL_RESCONTAINERAWARE(BoxGeometry3D);
	std::vector<std::shared_ptr<IDrawCallList>> _updatedDrawCallList;
	WRL::Wrappers::CriticalSection _containerCS;
	std::shared_ptr<FontManager> _fontManager;
};
END_NS_PLATFORM