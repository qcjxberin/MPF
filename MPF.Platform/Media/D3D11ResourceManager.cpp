//
// MPF Platform
// D3D11 Resource Manager
// ���ߣ�SunnyCase
// ����ʱ�䣺2016-08-28
//
#include "stdafx.h"
#include "D3D11ResourceManager.h"
using namespace WRL;
using namespace NS_PLATFORM;
using namespace DirectX;

namespace
{
	void EmplaceLine(std::vector<D3D11::StrokeVertex>& vertices, XMFLOAT2 startPoint, XMFLOAT2 endPoint, const XMVECTOR& normalStartVec, const XMVECTOR& normalEndVec)
	{
		XMFLOAT2 normalStart, normalStartOpp;
		XMFLOAT2 normalEnd, normalEndOpp;
		XMStoreFloat2(&normalStart, normalStartVec);
		XMStoreFloat2(&normalStartOpp, XMVectorScale(normalStartVec, -1.f));
		XMStoreFloat2(&normalEnd, normalEndVec);
		XMStoreFloat2(&normalEndOpp, XMVectorScale(normalEndVec, -1.f));

		// 1
		vertices.emplace_back(D3D11::StrokeVertex
		{
			{ startPoint.x, startPoint.y, 0.f },
			normalStart,{ 0, 0 }, D3D11::StrokeVertex::ST_Linear
		});
		vertices.emplace_back(D3D11::StrokeVertex
		{
			{ endPoint.x, endPoint.y, 0.f },
			normalEnd,{ 1.f, 1.f }, D3D11::StrokeVertex::ST_Linear
		});
		vertices.emplace_back(D3D11::StrokeVertex
		{
			{ endPoint.x, endPoint.y, 0.f },
			normalEndOpp,{ 1.f, 1.f }, D3D11::StrokeVertex::ST_Linear
		});

		// 2
		vertices.emplace_back(D3D11::StrokeVertex
		{
			{ endPoint.x, endPoint.y, 0.f },
			normalEndOpp,{ 1.f, 1.f }, D3D11::StrokeVertex::ST_Linear
		});
		vertices.emplace_back(D3D11::StrokeVertex
		{
			{ startPoint.x, startPoint.y, 0.f },
			normalStartOpp,{ 0, 0 }, D3D11::StrokeVertex::ST_Linear
		});
		vertices.emplace_back(D3D11::StrokeVertex
		{
			{ startPoint.x, startPoint.y, 0.f },
			normalStart,{ 0, 0 }, D3D11::StrokeVertex::ST_Linear
		});
	}

	void EmplaceArc(std::vector<D3D11::StrokeVertex>& vertices, XMFLOAT2 startPoint, XMFLOAT2 endPoint, float angle, const XMVECTOR& normalStartVec, const XMVECTOR& normalEndVec)
	{
		XMFLOAT2 normalStart, normalStartOpp;
		XMFLOAT2 normalEnd, normalEndOpp;
		XMStoreFloat2(&normalStart, normalStartVec);
		XMStoreFloat2(&normalStartOpp, XMVectorScale(normalStartVec, -1.f));
		XMStoreFloat2(&normalEnd, normalEndVec);
		XMStoreFloat2(&normalEndOpp, XMVectorScale(normalEndVec, -1.f));

		const auto dirVec = XMLoadFloat2(&XMFLOAT2{ endPoint.x - startPoint.x, endPoint.y - startPoint.y });
		const auto halfVec = XMVectorScale(dirVec, 0.5f);
		const auto centerDir = XMVector2Normalize(XMVector3Orthogonal(dirVec));
		const auto radian = XMConvertToRadians(angle);
		const auto radius = XMVector2Length(halfVec).m128_f32[0] / std::sin(radian / 2.f);
		const auto centerVec = XMLoadFloat2(&startPoint) + halfVec + centerDir * (radius * std::cos(radian / 2.f));
		const auto slopeLength = radius / std::cos(radian / 2.f) * 1.1f;
		const auto point2Vec = XMVector2Normalize(XMLoadFloat2(&startPoint) - centerVec) * slopeLength + centerVec;
		const auto point3Vec = XMVector2Normalize(XMLoadFloat2(&endPoint) - centerVec) * slopeLength + centerVec;
		XMFLOAT2 centerPoint, point2, point3;
		XMStoreFloat2(&centerPoint, centerVec);
		XMStoreFloat2(&point2, point2Vec);
		XMStoreFloat2(&point3, point3Vec);

		vertices.emplace_back(D3D11::StrokeVertex
		{
			{ centerPoint.x, centerPoint.y, 0.f },
			{ 0 ,1 },{ 0, 0 }, D3D11::StrokeVertex::ST_Arc
		});
		vertices.emplace_back(D3D11::StrokeVertex
		{
			{ point2.x, point2.y, 0.f },
			{ 0 ,1 },{ slopeLength / radius, 0 }, D3D11::StrokeVertex::ST_Arc
		});
		vertices.emplace_back(D3D11::StrokeVertex
		{
			{ point3.x, point3.y, 0.f },
			{ 3.f ,1 },{ slopeLength * std::cos(radian) / radius, slopeLength * std::sin(radian) / radius }, D3D11::StrokeVertex::ST_Arc
		});
	}

	void EmplaceQudraticBezier(std::vector<D3D11::StrokeVertex>& vertices, XMFLOAT2 startPoint, XMFLOAT2 endPoint, XMFLOAT2 control, const XMVECTOR& normalStartVec, const XMVECTOR& normalEndVec)
	{
		XMFLOAT2 normalStart, normalStartOpp;
		XMFLOAT2 normalEnd, normalEndOpp;
		XMStoreFloat2(&normalStart, normalStartVec);
		XMStoreFloat2(&normalStartOpp, XMVectorScale(normalStartVec, -1.f));
		XMStoreFloat2(&normalEnd, normalEndVec);
		XMStoreFloat2(&normalEndOpp, XMVectorScale(normalEndVec, -1.f));

		XMVECTOR aVec = XMLoadFloat2(&startPoint);
		XMVECTOR bVec = XMLoadFloat2(&endPoint);
		XMVECTOR cVec = XMLoadFloat2(&control);
		auto mVec = (aVec + bVec + cVec) / 3.f;
		auto minLen = std::min(XMVector2Length(aVec - mVec).m128_f32[0], XMVector2Length(bVec - mVec).m128_f32[0]);
		minLen = std::min(minLen, XMVector2Length(cVec - mVec).m128_f32[0]);

		XMFLOAT2 mPoint;
		XMStoreFloat2(&mPoint, mVec);

		vertices.emplace_back(D3D11::StrokeVertex
		{
			{ startPoint.x, startPoint.y, 0.f },
			mPoint,{ 0, 0 }, D3D11::StrokeVertex::ST_QuadraticBezier,{ minLen, 0 }
		});
		vertices.emplace_back(D3D11::StrokeVertex
		{
			{ control.x, control.y, 0.f },
			mPoint,{ 0.5f, 0 }, D3D11::StrokeVertex::ST_QuadraticBezier,{ minLen, 0 }
		});
		vertices.emplace_back(D3D11::StrokeVertex
		{
			{ endPoint.x, endPoint.y, 0.f },
			mPoint,{ 1, 1 }, D3D11::StrokeVertex::ST_QuadraticBezier,{ minLen, 0 }
		});
	}

	void SwapIfGeater(float& a, float& b)
	{
		if (a > b)
			std::swap(a, b);
	}
}

void ::NS_PLATFORM::Transform(std::vector<D3D11::StrokeVertex>& vertices, const LineGeometry& geometry)
{
	const auto dirVec = XMLoadFloat2(&XMFLOAT2{ geometry.Data.EndPoint.X - geometry.Data.StartPoint.X, geometry.Data.EndPoint.Y - geometry.Data.StartPoint.Y });
	const auto normalVec = XMVector2Normalize(XMVector2Orthogonal(dirVec));

	EmplaceLine(vertices, { geometry.Data.StartPoint.X, geometry.Data.StartPoint.Y }, { geometry.Data.EndPoint.X, geometry.Data.EndPoint.Y },
		normalVec, normalVec);
}

void ::NS_PLATFORM::Transform(std::vector<D3D11::StrokeVertex>& vertices, const RectangleGeometry& geometry)
{
	auto leftTopPoint = geometry.Data.LeftTop;
	auto rightBottomPoint = geometry.Data.RightBottom;
	SwapIfGeater(leftTopPoint.X, rightBottomPoint.X);
	SwapIfGeater(leftTopPoint.Y, rightBottomPoint.Y);

	XMFLOAT2 leftTop{ leftTopPoint.X, leftTopPoint.Y };
	XMFLOAT2 rightTop{ rightBottomPoint.X, leftTopPoint.Y };
	XMFLOAT2 rightBottom{ rightBottomPoint.X, rightBottomPoint.Y };
	XMFLOAT2 leftBottom{ leftTopPoint.X, rightBottomPoint.Y };

	const auto ltDirVec = XMLoadFloat2(&XMFLOAT2{ -1.f, -1.f });
	const auto rtDirVec = XMLoadFloat2(&XMFLOAT2{ 1.f, -1.f });
	const auto lbDirVec = XMLoadFloat2(&XMFLOAT2{ -1.f, 1.f });
	const auto rbDirVec = XMLoadFloat2(&XMFLOAT2{ 1.f, 1.f });

	EmplaceLine(vertices, leftTop, rightTop, ltDirVec, rtDirVec);
	EmplaceLine(vertices, rightTop, rightBottom, rtDirVec, rbDirVec);
	EmplaceLine(vertices, rightBottom, leftBottom, rbDirVec, lbDirVec);
	EmplaceLine(vertices, leftBottom, leftTop, lbDirVec, ltDirVec);
}

void ::NS_PLATFORM::Transform(std::vector<D3D11::StrokeVertex>& vertices, const PathGeometry& geometry)
{
	using namespace PathGeometrySegments;

	XMFLOAT2 lastPoint{ 0,0 };
	for (auto&& seg : geometry.Segments)
	{
		switch (seg.Operation)
		{
		case MoveTo:
		{
			const auto& data = seg.Data.MoveTo;
			lastPoint = { data.Point.X, data.Point.Y };
		}
		break;
		case LineTo:
		{
			const auto& data = seg.Data.LineTo;
			XMFLOAT2 endPoint(data.Point.X, data.Point.Y);

			const auto dirVec = XMLoadFloat2(&XMFLOAT2{ endPoint.x - lastPoint.x, endPoint.y - lastPoint.y });
			const auto normalVec = XMVector2Normalize(XMVector2Orthogonal(dirVec));

			EmplaceLine(vertices, lastPoint, endPoint, normalVec, normalVec);
			lastPoint = endPoint;
		}
		break;
		case ArcTo:
		{
			const auto& data = seg.Data.ArcTo;
			XMFLOAT2 endPoint(data.Point.X, data.Point.Y);

			const auto dirVec = XMLoadFloat2(&XMFLOAT2{ endPoint.x - lastPoint.x, endPoint.y - lastPoint.y });
			const auto normalVec = XMVector2Normalize(XMVector2Orthogonal(dirVec));

			EmplaceArc(vertices, lastPoint, endPoint, data.Angle, normalVec, normalVec);
			lastPoint = endPoint;
		}
		break;
		case QuadraticBezierTo:
		{
			const auto& data = seg.Data.QuadraticBezierTo;
			XMFLOAT2 endPoint(data.Point.X, data.Point.Y);

			const auto dirVec = XMLoadFloat2(&XMFLOAT2{ endPoint.x - lastPoint.x, endPoint.y - lastPoint.y });
			const auto normalVec = XMVector2Normalize(XMVector2Orthogonal(dirVec));

			EmplaceQudraticBezier(vertices, lastPoint, endPoint, { data.Control.X, data.Control.Y }, normalVec, normalVec);
			lastPoint = endPoint;
		}
		break;
		default:
			break;
		}
	}
}

namespace
{
	class DrawCallList : public IDrawCallList
	{
		struct MyStrokeDrawCall : public StorkeRenderCall
		{
			DirectX::XMFLOAT4X4 Transform;
		};
	public:
		DrawCallList(ID3D11DeviceContext* deviceContext, SwapChainUpdateContext& updateContext, D3D11ResourceManager* resMgr, RenderCommandBuffer* rcb)
			:_deviceContext(deviceContext), _resMgr(resMgr), _rcb(rcb), _updateContext(updateContext)
		{

		}

		// ͨ�� IDrawCallList �̳�
		virtual void Draw(const DirectX::XMFLOAT4X4& modelTransform) override
		{
			using namespace D3D11;

			{
				auto model = _updateContext.Model.Map(_deviceContext.Get());
				model->Model = modelTransform;
			}
			float constants[4] = { 0 };
			UINT offset = 0;
			for (auto&& rc : _strokeRenderCalls)
			{
				auto vb = _resMgr->GetVertexBuffer(rc);
				_deviceContext->IASetVertexBuffers(0, 1, &vb, &rc.Stride, &offset);

				{
					auto geo = _updateContext.Geometry.Map(_deviceContext.Get());
					ThrowIfNot(memcpy_s(geo->Color, sizeof(geo->Color), rc.Color, sizeof(rc.Color)) == 0, L"Cannot copy color.");
					geo->Thickness = rc.Thickness;
					geo->Transform = rc.Transform;
				}
				_deviceContext->Draw(rc.VertexCount, rc.StartVertex);
			}
		}

		virtual void Update() override
		{
			Update(false);
		}

		void Initialize()
		{
			Update(true);
		}
	private:
		// ͨ�� IDrawCallList �̳�
		void PushGeometryDrawCall(IResource* resource, IResource* pen, const DirectX::XMFLOAT4X4 transform)
		{
			if (pen)
			{
				MyStrokeDrawCall rc;
				auto& penObj = _resMgr->GetPen(static_cast<ResourceRef*>(pen)->GetHandle());
				rc.Thickness = penObj.Thickness;
				if (penObj.Brush)
				{
					auto& brushObj = _resMgr->GetBrush(penObj.Brush->GetHandle());
					if (typeid(brushObj) == typeid(Brush&))
					{
						auto& color = static_cast<SolidColorBrush&>(brushObj).Color;
						rc.Color[0] = color.R;
						rc.Color[1] = color.G;
						rc.Color[2] = color.B;
						rc.Color[3] = color.A;
					}
					if (_resMgr->TryGet(resource, rc))
					{
						rc.Transform = transform;
						_strokeRenderCalls.emplace_back(rc);
					}
					else
					{
						assert(false && "Geometry not found.");
					}
				}
			}
		}

		void Update(bool addResDependent)
		{
			_strokeRenderCalls.clear();
			for (auto&& geoRef : _rcb->GetGeometries())
			{
				PushGeometryDrawCall(geoRef.Geometry.Get(), geoRef.Pen.Get(), geoRef.Transform);
				if (addResDependent)
				{
					auto me = shared_from_this();
					_resMgr->AddDependentDrawCallList(me, geoRef.Geometry.Get());
					_resMgr->AddDependentDrawCallList(me, geoRef.Pen.Get());
				}
			}
		}
	private:
		ComPtr<ID3D11DeviceContext> _deviceContext;
		SwapChainUpdateContext& _updateContext;
		ComPtr<D3D11ResourceManager> _resMgr;
		ComPtr<RenderCommandBuffer> _rcb;
		std::vector<MyStrokeDrawCall> _strokeRenderCalls;
	};
}

#define CTOR_IMPL1(T) _trc##T##(_strokeVBMgr)

D3D11ResourceManager::D3D11ResourceManager(ID3D11Device* device, ID3D11DeviceContext* deviceContext, SwapChainUpdateContext& updateContext)
	:_device(device), _deviceContext(deviceContext), _updateContext(updateContext), _strokeVBMgr(device, sizeof(D3D11::StrokeVertex)), CTOR_IMPL1(LineGeometry), CTOR_IMPL1(RectangleGeometry), CTOR_IMPL1(PathGeometry)
{
}

void D3D11ResourceManager::UpdateOverride()
{
	_strokeVBMgr.Upload(_deviceContext.Get());
}

std::shared_ptr<IDrawCallList> D3D11ResourceManager::CreateDrawCallList(RenderCommandBuffer* rcb)
{
	auto ret = std::make_shared<DrawCallList>(_deviceContext.Get(), _updateContext, this, rcb);
	ret->Initialize();
	return ret;
}

#define TRYGET_IMPL1(T)										 \
	case RT_##T:											 \
		return _trc##T.TryGet(resRef->GetHandle(), rc);

bool D3D11ResourceManager::TryGet(IResource* res, StorkeRenderCall& rc) const
{
	auto resRef = static_cast<ResourceRef*>(res);
	switch (resRef->GetType())
	{
		TRYGET_IMPL1(LineGeometry);
		TRYGET_IMPL1(RectangleGeometry);
		TRYGET_IMPL1(PathGeometry);
	default:
		break;
	}
	return false;
}