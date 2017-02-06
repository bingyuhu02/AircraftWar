#include "Draw.h"
#include "stdafx.h"

Draw::Draw()
	: m_pMyPlane(NULL)
	, m_pBackGround(NULL)
	, m_pRenderTarget(NULL)
	, m_pWICFactory(NULL)
{
}

Draw::~Draw()
{
	//delete m_pMyPlane;
	//delete m_pBackGround;
}
void Draw::Init(ID2D1HwndRenderTarget *pRenderTarget, IWICImagingFactory *pWICFactory)
{
	Assert(pRenderTarget != nullptr && pRenderTarget != NULL);
	Assert(pWICFactory != nullptr && pWICFactory != NULL);

	m_pRenderTarget = pRenderTarget;
	m_pWICFactory = pWICFactory;

	LoadBitmapFromFile(m_pRenderTarget, m_pWICFactory, L"bluesky.jpg", 0, 0, &m_pBackGround);
	LoadBitmapFromFile(m_pRenderTarget, m_pWICFactory, L"bullet.png", 0, 0, &m_pMyBullet);
	LoadBitmapFromFile(m_pRenderTarget, m_pWICFactory, L"bitmap.png", 0, 0, &m_pMyPlane);
	LoadBitmapFromFile(m_pRenderTarget, m_pWICFactory, L"enemy.png", 0, 0, &m_pMyEnemy);
	LoadBitmapFromFile(m_pRenderTarget, m_pWICFactory, L"redline.jpg", 0, 0, &m_pRedline);

	LoadBitmapFromFile(m_pRenderTarget, m_pWICFactory, L"Number1.png", 0, 0, &m_pOne);
	LoadBitmapFromFile(m_pRenderTarget, m_pWICFactory, L"Number2.png", 0, 0, &m_pTwo);
	LoadBitmapFromFile(m_pRenderTarget, m_pWICFactory, L"Number3.png", 0, 0, &m_pThree);
	LoadBitmapFromFile(m_pRenderTarget, m_pWICFactory, L"Go.png", 0, 0, &m_pGo);
	LoadBitmapFromFile(m_pRenderTarget, m_pWICFactory, L"GameOver.png", 0, 0, &m_pGameOver);

	m_pScreenVec.resize(m_pic_num);
	for (size_t i = 0; i < m_pic_num; i++)
	{
		WCHAR s[100];
		swprintf_s(s, 100, L"background\\%d.png", i);
		LoadBitmapFromFile(m_pRenderTarget, m_pWICFactory, s, 0, 0, &(m_pScreenVec[i]));
	}
	m_background_counter = 0;
}
void Draw::drawMyPlane(unsigned int x, unsigned int y, int width, int height)
{
	m_pRenderTarget->DrawBitmap(
		m_pMyPlane,
		D2D1::RectF(x, y, x + width, y + height));
}

void Draw::drawMyBackGround(unsigned int width, unsigned int height)
{
	m_pRenderTarget->DrawBitmap(
		m_pBackGround,
		D2D1::RectF(0, 0, width, height));
}

void Draw::drawOne(int width, int height)
{
	m_pRenderTarget->DrawBitmap(
		m_pOne,
		D2D1::RectF(width / 2 - 80, height / 2 - 80, width / 2 + 80, height / 2 + 80));
}
void Draw::drawTwo(int width, int height)
{
	m_pRenderTarget->DrawBitmap(
		m_pTwo,
		D2D1::RectF(width / 2 - 80, height / 2 - 80, width / 2 + 80, height / 2 + 80));
}
void Draw::drawThree(int width, int height)
{
	m_pRenderTarget->DrawBitmap(
		m_pThree,
		D2D1::RectF(width / 2 - 80, height / 2 - 80, width / 2 + 80, height / 2 + 80));
}
void Draw::drawGo(int width, int height)
{
	m_pRenderTarget->DrawBitmap(
		m_pGo,
		D2D1::RectF(width / 2 - 80, height / 2 - 80, width / 2 + 80, height / 2 + 80));
}

void Draw::drawGameOver(int width, int height)
{
	m_pRenderTarget->DrawBitmap(
		m_pGameOver,
		D2D1::RectF(width / 2 - 200, height / 2 - 200, width / 2 + 200, height / 2 + 200));
}

void Draw::drawNextBackground(unsigned int width, unsigned int height)
{
	int step = height / m_pic_num;
	float stepf = float(height) / float(m_pic_num);

	size_t i;
	for (i = 0; i < m_pic_num - m_background_counter; i++)
	{
		m_pRenderTarget->DrawBitmap(
			m_pScreenVec[i + m_background_counter],
			D2D1::RectF(0, i * stepf, width, (i+1) * stepf + 1));
	}
	for (; i < m_pic_num; i++)
	{
		m_pRenderTarget->DrawBitmap(
			m_pScreenVec[i- m_pic_num + m_background_counter],
			D2D1::RectF(0, int(i * stepf), width, int((i + 1) * stepf) + 1));
	}

	if (m_background_counter == 0)
	{
		m_background_counter = m_pic_num - 1;
	}
	m_background_counter--;
}

void Draw::drawMyBullet(unsigned int x, unsigned int y, int width, int height)
{
	m_pRenderTarget->DrawBitmap(
		m_pMyBullet,
		D2D1::RectF(x, y, x + width, y + height));
}

void Draw::drawMyEnemy(unsigned int x, unsigned int y, int width, int height)
{
	m_pRenderTarget->DrawBitmap(
		m_pMyEnemy,
		D2D1::RectF(x, y, x + width, y + height));
}

void Draw::drawRedline(unsigned int x, unsigned int y, int width, int height)
{
	m_pRenderTarget->DrawBitmap(
		m_pRedline,
		D2D1::RectF(x, y, x + width, y + height));
}


HRESULT Draw::LoadBitmapFromFile(
	ID2D1RenderTarget *pRenderTarget,
	IWICImagingFactory *pIWICFactory,
	LPCWSTR uri,
	UINT width,
	UINT height,
	ID2D1Bitmap **ppBitmap)
{
	IWICBitmapDecoder *pDecoder = NULL;
	IWICBitmapFrameDecode *pSource = NULL;
	IWICStream *pStream = NULL;
	IWICFormatConverter *pConverter = NULL;
	IWICBitmapScaler *pScaler = NULL;

	HRESULT hr = pIWICFactory->CreateDecoderFromFilename(
		uri,
		NULL,
		GENERIC_READ,
		WICDecodeMetadataCacheOnLoad,
		&pDecoder
	);

	if (SUCCEEDED(hr))
	{
		hr = pDecoder->GetFrame(0, &pSource);
	}
	if (SUCCEEDED(hr))
	{
		hr = pIWICFactory->CreateFormatConverter(&pConverter);
	}

	if (SUCCEEDED(hr))
	{
		if (width != 0 || height != 0)
		{
			UINT originalWidth, originalHeight;
			hr = pSource->GetSize(&originalWidth, &originalHeight);
			if (SUCCEEDED(hr))
			{
				if (width == 0)
				{
					FLOAT scalar = static_cast<FLOAT>(height) / static_cast<FLOAT>(originalHeight);
					width = static_cast<UINT>(scalar * static_cast<FLOAT>(originalWidth));
				}
				else if (height == 0)
				{
					FLOAT scalar = static_cast<FLOAT>(width) / static_cast<FLOAT>(originalWidth);
					height = static_cast<UINT>(scalar * static_cast<FLOAT>(originalHeight));
				}

				hr = pIWICFactory->CreateBitmapScaler(&pScaler);
				if (SUCCEEDED(hr))
				{
					hr = pScaler->Initialize(
						pSource,
						width,
						height,
						WICBitmapInterpolationModeCubic
					);
				}
				if (SUCCEEDED(hr))
				{
					hr = pConverter->Initialize(
						pScaler,
						GUID_WICPixelFormat32bppPBGRA,
						WICBitmapDitherTypeNone,
						NULL,
						0.f,
						WICBitmapPaletteTypeMedianCut
					);
				}
			}
		}
		else // Don't scale the image.
		{
			hr = pConverter->Initialize(
				pSource,
				GUID_WICPixelFormat32bppPBGRA,
				WICBitmapDitherTypeNone,
				NULL,
				0.f,
				WICBitmapPaletteTypeMedianCut
			);
		}
	}
	if (SUCCEEDED(hr))
	{

		// Create a Direct2D bitmap from the WIC bitmap.
		hr = pRenderTarget->CreateBitmapFromWicBitmap(
			pConverter,
			NULL,
			ppBitmap
		);
	}

	SafeRelease(pDecoder);
	SafeRelease(pSource);
	SafeRelease(pStream);
	SafeRelease(pConverter);
	SafeRelease(pScaler);

	return hr;
}

