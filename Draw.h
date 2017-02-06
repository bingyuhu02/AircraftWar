#pragma once
#include"stdafx.h"
#include <vector>


class Draw
{
private:
	ID2D1Bitmap*			m_pMyPlane;
	ID2D1Bitmap*			m_pMyBullet;
	ID2D1Bitmap*			m_pBackGround;
	ID2D1Bitmap*			m_pMyEnemy;
	ID2D1Bitmap*			m_pRedline;

	ID2D1Bitmap*			m_pThree;
	ID2D1Bitmap*			m_pTwo;
	ID2D1Bitmap*			m_pOne;
	ID2D1Bitmap*			m_pGo;
	ID2D1Bitmap*			m_pGameOver;

	std::vector<ID2D1Bitmap*> m_pScreenVec;

	ID2D1HwndRenderTarget*  m_pRenderTarget;
	IWICImagingFactory*		m_pWICFactory;

	long unsigned int		m_background_counter;


	const int				m_pic_num = 169;

public:
	Draw();
	~Draw();

	void Init(ID2D1HwndRenderTarget *pRenderTarget, IWICImagingFactory *pWICFactory);
	void drawMyPlane(unsigned int x, unsigned int y, int width, int height);
	void drawMyBackGround(unsigned int width, unsigned int height);
	void drawMyBullet(unsigned int x, unsigned int y, int width, int height);
	void drawMyEnemy(unsigned int x, unsigned int y, int width, int height);
	void drawRedline(unsigned int x, unsigned int y, int width, int height);

	void drawOne(int width, int height);
	void drawTwo(int width, int height);
	void drawThree(int width, int height);
	void drawGo(int width, int height);
	void drawGameOver(int width, int height);


	void drawNextBackground(unsigned int width, unsigned int height);

	HRESULT LoadBitmapFromFile(
		ID2D1RenderTarget *pRenderTarget,
		IWICImagingFactory *pIWICFactory,
		LPCWSTR uri,
		UINT width,
		UINT height,
		ID2D1Bitmap **ppBitmap);
};