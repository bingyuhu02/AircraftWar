#include "stdafx.h"
#include <strsafe.h>
#include "resource.h"
#include "BodyBasics.h"
#include <cmath>
#include <stdlib.h>
#include <time.h>
#include <random>
#include <vector>

static const float c_JointThickness = 3.0f;
static const float c_TrackedBoneThickness = 6.0f;
static const float c_InferredBoneThickness = 1.0f;
static const float c_HandSize = 30.0f;

/// <summary>
/// Entry point for the application
/// </summary>
/// <param name="hInstance">handle to the application instance</param>
/// <param name="hPrevInstance">always 0</param>
/// <param name="lpCmdLine">command line arguments</param>
/// <param name="nCmdShow">whether to display minimized, maximized, or normally</param>
/// <returns>status</returns>
int APIENTRY wWinMain(    
	_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nShowCmd
)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    CBodyBasics application;
    application.Run(hInstance, nShowCmd);
}

/// <summary>
/// Constructor
/// </summary>
CBodyBasics::CBodyBasics() :
    m_hWnd(NULL),
    m_nStartTime(0),
    m_nLastCounter(0),
    m_nFramesSinceUpdate(0),
    m_fFreq(0),
    m_nNextStatusTime(0LL),
    m_pKinectSensor(NULL),
    m_pCoordinateMapper(NULL),
    m_pBodyFrameReader(NULL),
    m_pD2DFactory(NULL),
    m_pRenderTarget(NULL),
    m_pBrushJointTracked(NULL),
    m_pBrushJointInferred(NULL),
    m_pBrushBoneTracked(NULL),
    m_pBrushBoneInferred(NULL),
    m_pBrushHandClosed(NULL),
    m_pBrushHandOpen(NULL),
    m_pBrushHandLasso(NULL),
	m_pBrushScore(NULL),
	m_lastSkeletonID(-1),
	m_pWICFactory(NULL),
	m_pDraw(NULL),
	m_bullet_count(0),
	m_pDWriteFactory (NULL),
	m_pTextFormat(NULL)
{
    LARGE_INTEGER qpf = {0};
    if (QueryPerformanceFrequency(&qpf))
    {
        m_fFreq = double(qpf.QuadPart);

    }

	srand(time(NULL));

	m_pDraw = new Draw();
	m_plane_size_width = 100;
	m_plane_size_height = 100;
	m_bullet_count = 0;


	m_enemy_count = 0;
	m_enemy_next_count = 100;
	m_enemy_pass = 0;


	m_score = 0;
	m_redline_counter = 0;
	m_init_counter = 0;
	m_isTracking = false;
	m_isGameStart = false;
	m_isGameOver = false;
}
  

/// <summary>
/// Destructor
/// </summary>
CBodyBasics::~CBodyBasics()
{
    DiscardDirect2DResources();

    // clean up Direct2D
    SafeRelease(m_pD2DFactory);

    // done with body frame reader
    SafeRelease(m_pBodyFrameReader);

    // done with coordinate mapper
    SafeRelease(m_pCoordinateMapper);

    // close the Kinect Sensor
    if (m_pKinectSensor)
    {
        m_pKinectSensor->Close();
    }

    SafeRelease(m_pKinectSensor);
	delete m_pDraw;
}

/// <summary>
/// Creates the main window and begins processing
/// </summary>
/// <param name="hInstance">handle to the application instance</param>
/// <param name="nCmdShow">whether to display minimized, maximized, or normally</param>
int CBodyBasics::Run(HINSTANCE hInstance, int nCmdShow)
{
    MSG       msg = {0};
    WNDCLASS  wc;

    // Dialog custom window class
    ZeroMemory(&wc, sizeof(wc));
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.cbWndExtra    = DLGWINDOWEXTRA;
    wc.hCursor       = LoadCursorW(NULL, IDC_ARROW);
    wc.hIcon         = LoadIconW(hInstance, MAKEINTRESOURCE(IDI_APP));
    wc.lpfnWndProc   = DefDlgProcW;
    wc.lpszClassName = L"BodyBasicsAppDlgWndClass";

    if (!RegisterClassW(&wc))
    {
        return 0;
    }

    // Create main application window
    HWND hWndApp = CreateDialogParamW(
        NULL,
        MAKEINTRESOURCE(IDD_APP),
        NULL,
        (DLGPROC)CBodyBasics::MessageRouter, 
        reinterpret_cast<LPARAM>(this));

    // Show window
    ShowWindow(hWndApp, nCmdShow);

	RECT rct;
	GetClientRect(GetDlgItem(m_hWnd, IDC_GAMEVIEW), &rct);
	m_game_view_width = rct.right;
	m_game_view_height = rct.bottom;


	m_pPlane = new Plane(m_game_view_width / 2 - m_plane_size_width / 2,
		m_game_view_height - m_plane_size_height,
		m_plane_size_width,
		m_plane_size_height,
		m_game_view_width);

    // Main message loop
    while (WM_QUIT != msg.message)
    {
        Update();

        while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
        {
            // If a dialog message will be taken care of by the dialog proc
            if (hWndApp && IsDialogMessageW(hWndApp, &msg))
            {
                continue;
            }

            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    return static_cast<int>(msg.wParam);
}

/// <summary>
/// Main processing function
/// </summary>
void CBodyBasics::Update()
{
    if (!m_pBodyFrameReader)
    {
        return;
    }

    IBodyFrame* pBodyFrame = NULL;

    HRESULT hr = m_pBodyFrameReader->AcquireLatestFrame(&pBodyFrame);

    if (SUCCEEDED(hr))
    {
        INT64 nTime = 0;

        hr = pBodyFrame->get_RelativeTime(&nTime);

        IBody* ppBodies[BODY_COUNT] = {0};

        if (SUCCEEDED(hr))
        {
            hr = pBodyFrame->GetAndRefreshBodyData(_countof(ppBodies), ppBodies);
        }

        if (SUCCEEDED(hr))
        {
            ProcessBody(nTime, BODY_COUNT, ppBodies);
        }

        for (int i = 0; i < _countof(ppBodies); ++i)
        {
            SafeRelease(ppBodies[i]);
        }
    }

    SafeRelease(pBodyFrame);
}

/// <summary>
/// Handles window messages, passes most to the class instance to handle
/// </summary>
/// <param name="hWnd">window message is for</param>
/// <param name="uMsg">message</param>
/// <param name="wParam">message data</param>
/// <param name="lParam">additional message data</param>
/// <returns>result of message processing</returns>
LRESULT CALLBACK CBodyBasics::MessageRouter(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CBodyBasics* pThis = NULL;
    
    if (WM_INITDIALOG == uMsg)
    {
        pThis = reinterpret_cast<CBodyBasics*>(lParam);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    }
    else
    {
        pThis = reinterpret_cast<CBodyBasics*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
    }

    if (pThis)
    {
        return pThis->DlgProc(hWnd, uMsg, wParam, lParam);
    }

    return 0;
}

/// <summary>
/// Handle windows messages for the class instance
/// </summary>
/// <param name="hWnd">window message is for</param>
/// <param name="uMsg">message</param>
/// <param name="wParam">message data</param>
/// <param name="lParam">additional message data</param>
/// <returns>result of message processing</returns>
LRESULT CALLBACK CBodyBasics::DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);

    switch (message)
    {
        case WM_INITDIALOG:
        {
            // Bind application window handle
            m_hWnd = hWnd;

            // Init Direct2D
            D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);

            // Get and initialize the default Kinect sensor
            InitializeDefaultSensor();
        }
        break;

		// Todo: Handle windows message


        // If the titlebar X is clicked, destroy app
        case WM_CLOSE:
            DestroyWindow(hWnd);
            break;

        case WM_DESTROY:
            // Quit the main message pump
            PostQuitMessage(0);
            break;
    }

    return FALSE;
}

/// <summary>
/// Initializes the default Kinect sensor
/// </summary>
/// <returns>indicates success or failure</returns>
HRESULT CBodyBasics::InitializeDefaultSensor()
{
    HRESULT hr;

    hr = GetDefaultKinectSensor(&m_pKinectSensor);
    if (FAILED(hr))
    {
        return hr;
    }

    if (m_pKinectSensor)
    {
        // Initialize the Kinect and get coordinate mapper and the body reader
        IBodyFrameSource* pBodyFrameSource = NULL;

        hr = m_pKinectSensor->Open();

        if (SUCCEEDED(hr))
        {
            hr = m_pKinectSensor->get_CoordinateMapper(&m_pCoordinateMapper);
        }

        if (SUCCEEDED(hr))
        {
            hr = m_pKinectSensor->get_BodyFrameSource(&pBodyFrameSource);
        }

        if (SUCCEEDED(hr))
        {
            hr = pBodyFrameSource->OpenReader(&m_pBodyFrameReader);
        }

        SafeRelease(pBodyFrameSource);
    }

    if (!m_pKinectSensor || FAILED(hr))
    {
        SetStatusMessage(L"No ready Kinect found!", 10000, true);
        return E_FAIL;
    }

    return hr;
}

/// <summary>
/// Handle new body data
/// <param name="nTime">timestamp of frame</param>
/// <param name="nBodyCount">body data count</param>
/// <param name="ppBodies">body data in frame</param>
/// </summary>
void CBodyBasics::ProcessBody(INT64 nTime, int nBodyCount, IBody** ppBodies)
{
    if (m_hWnd)
    {
        HRESULT hr = EnsureDirect2DResources();

		double arm_degree = 0;
        if (SUCCEEDED(hr) && m_pRenderTarget && m_pCoordinateMapper)
        {
            m_pRenderTarget->BeginDraw();
            m_pRenderTarget->Clear();

            RECT rct;
            GetClientRect(GetDlgItem(m_hWnd, IDC_VIDEOVIEW), &rct);
            int width = rct.right;
            int height = rct.bottom;
			// game view
			GetClientRect(GetDlgItem(m_hWnd, IDC_GAMEVIEW), &rct);
			int game_view_width = rct.right;
			int game_view_height = rct.bottom;



			IBody* pBody = nullptr;
	        if (m_lastSkeletonID >= 0)
	        {
		        // tracking skeleton
				pBody = ppBodies[m_lastSkeletonID];
				if (pBody)
				{
					BOOLEAN bTracked = false;
					hr = pBody->get_IsTracked(&bTracked);

					if (SUCCEEDED(hr) && bTracked)
					{
						// tracking same skeleton as last frame
					}
					else
					{
						m_lastSkeletonID = -1;
					}
				}
	        }

	        if (m_lastSkeletonID < 0)
	        {
				// Not tracking same skeleton as last frame
				// Try to init a new skeleton

				for (int i = 0; i < nBodyCount; ++i)
				{
					pBody = ppBodies[i];
					if (pBody)
					{
						BOOLEAN bTracked = false;
						hr = pBody->get_IsTracked(&bTracked);

						if (SUCCEEDED(hr) && bTracked)
						{
							m_lastSkeletonID = i;
							break;
						}
					}
				}
	        }

	        if (m_lastSkeletonID >= 0)
	        {
				Joint joints[JointType_Count];
				D2D1_POINT_2F jointPoints[JointType_Count];
				HandState leftHandState = HandState_Unknown;
				HandState rightHandState = HandState_Unknown;

				pBody->get_HandLeftState(&leftHandState);
				pBody->get_HandRightState(&rightHandState);

				hr = pBody->GetJoints(_countof(joints), joints);
				if (SUCCEEDED(hr))
				{
					for (int j = 0; j < _countof(joints); ++j)
					{
						jointPoints[j] = BodyToScreen(joints[j].Position, width, height);
					}

					DrawBody(joints, jointPoints);

					DrawHand(leftHandState, jointPoints[JointType_HandLeft]);
					DrawHand(rightHandState, jointPoints[JointType_HandRight]);

					// Todo: Draw game view 
					arm_degree = DrawGameViewActive(joints, jointPoints, game_view_width, game_view_height);
				}
	        }
			else
			{
				DrawGameViewInactive(game_view_width, game_view_height);
			}

            //for (int i = 0; i < nBodyCount; ++i)
            //{
            //    IBody* pBody = ppBodies[i];
            //    if (pBody)
            //    {
            //        BOOLEAN bTracked = false;
            //        hr = pBody->get_IsTracked(&bTracked);
			//
            //        if (SUCCEEDED(hr) && bTracked)
            //        {
            //            Joint joints[JointType_Count]; 
            //            D2D1_POINT_2F jointPoints[JointType_Count];
            //            HandState leftHandState = HandState_Unknown;
            //            HandState rightHandState = HandState_Unknown;
			//
            //            pBody->get_HandLeftState(&leftHandState);
            //            pBody->get_HandRightState(&rightHandState);
			//
            //            hr = pBody->GetJoints(_countof(joints), joints);
            //            if (SUCCEEDED(hr))
            //            {
            //                for (int j = 0; j < _countof(joints); ++j)
            //                {
            //                    jointPoints[j] = BodyToScreen(joints[j].Position, width, height);
            //                }
			//
            //                DrawBody(joints, jointPoints);
			//
            //                DrawHand(leftHandState, jointPoints[JointType_HandLeft]);
            //                DrawHand(rightHandState, jointPoints[JointType_HandRight]);
            //            }
            //        }
            //    }
            //}

            hr = m_pRenderTarget->EndDraw();

            // Device lost, need to recreate the render target
            // We'll dispose it now and retry drawing
            if (D2DERR_RECREATE_TARGET == hr)
            {
                hr = S_OK;
                DiscardDirect2DResources();
            }
        }

        if (!m_nStartTime)
        {
            m_nStartTime = nTime;
        }

        double fps = 0.0;

        LARGE_INTEGER qpcNow = {0};
        if (m_fFreq)
        {
            if (QueryPerformanceCounter(&qpcNow))
            {
                if (m_nLastCounter)
                {
                    m_nFramesSinceUpdate++;
                    fps = m_fFreq * m_nFramesSinceUpdate / double(qpcNow.QuadPart - m_nLastCounter);
                }
            }
        }

        WCHAR szStatusMessage[64];
        StringCchPrintf(szStatusMessage, _countof(szStatusMessage), L" DEGREE = %0.2f    FPS = %0.2f", arm_degree,  fps);

        if (SetStatusMessage(szStatusMessage, 1000, false))
        {
            m_nLastCounter = qpcNow.QuadPart;
            m_nFramesSinceUpdate = 0;
        }
    }
}

/// <summary>
/// Set the status bar message
/// </summary>
/// <param name="szMessage">message to display</param>
/// <param name="showTimeMsec">time in milliseconds to ignore future status messages</param>
/// <param name="bForce">force status update</param>
bool CBodyBasics::SetStatusMessage(_In_z_ WCHAR* szMessage, DWORD nShowTimeMsec, bool bForce)
{
    INT64 now = GetTickCount64();

    if (m_hWnd && (bForce || (m_nNextStatusTime <= now)))
    {
        SetDlgItemText(m_hWnd, IDC_STATUS, szMessage);
        m_nNextStatusTime = now + nShowTimeMsec;

        return true;
    }

    return false;
}

/// <summary>
/// Ensure necessary Direct2d resources are created
/// </summary>
/// <returns>S_OK if successful, otherwise an error code</returns>
HRESULT CBodyBasics::EnsureDirect2DResources()
{
    HRESULT hr = S_OK;

    if (m_pD2DFactory && !m_pRenderTarget)
    {
        RECT rc;
        GetWindowRect(GetDlgItem(m_hWnd, IDC_VIDEOVIEW), &rc);  

        int width = rc.right - rc.left;
        int height = rc.bottom - rc.top;
        D2D1_SIZE_U size = D2D1::SizeU(width, height);
        D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties();
        rtProps.pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE);
        rtProps.usage = D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE;

        // Create a Hwnd render target, in order to render to the window set in initialize
        hr = m_pD2DFactory->CreateHwndRenderTarget(
            rtProps,
            D2D1::HwndRenderTargetProperties(GetDlgItem(m_hWnd, IDC_VIDEOVIEW), size),
            &m_pRenderTarget
        );

        if (FAILED(hr))
        {
            SetStatusMessage(L"Couldn't create Direct2D render target!", 10000, true);
            return hr;
        }

		if (m_pWICFactory == NULL && SUCCEEDED(hr))
		{
			if (!SUCCEEDED(
				CoCreateInstance(
					CLSID_WICImagingFactory,
					NULL,
					CLSCTX_INPROC_SERVER,
					IID_PPV_ARGS(&m_pWICFactory)
				)
			))
				return FALSE;
		}

        // light green
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.27f, 0.75f, 0.27f), &m_pBrushJointTracked);

        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Yellow, 1.0f), &m_pBrushJointInferred);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Green, 1.0f), &m_pBrushBoneTracked);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Gray, 1.0f), &m_pBrushBoneInferred);

        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red, 0.5f), &m_pBrushHandClosed);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Green, 0.5f), &m_pBrushHandOpen);
        m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Blue, 0.5f), &m_pBrushHandLasso);


		m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red, 0.5f), &m_pBrushScore);


		// Create a Hwnd render target, in order to render to the window set in initialize
		GetWindowRect(GetDlgItem(m_hWnd, IDC_GAMEVIEW), &rc);

		width = rc.right - rc.left;
		height = rc.bottom - rc.top;
		size = D2D1::SizeU(width, height);

		hr = m_pD2DFactory->CreateHwndRenderTarget(
			rtProps,
			D2D1::HwndRenderTargetProperties(GetDlgItem(m_hWnd, IDC_GAMEVIEW), size),
			&m_pRenderGame
		);

		// create dwrite factory  
		DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(IDWriteFactory),
			reinterpret_cast<IUnknown**>(&m_pDWriteFactory));

		//create text format  
		hr = m_pDWriteFactory->CreateTextFormat(
			L"Arial",
			NULL,
			DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			20.0f * 96.0f / 72.0f,
			L"en-US",
			&m_pTextFormat
		);

		if (FAILED(hr))
		{
			SetStatusMessage(L"Couldn't create Direct2D render game target!", 10000, true);
			return hr;
		}

		// light green
		m_pRenderGame->CreateSolidColorBrush(D2D1::ColorF(0.27f, 0.75f, 0.27f), &m_pBrushTriangle);

		m_pDraw->Init(m_pRenderGame, m_pWICFactory);

    }

    return hr;
}

/// <summary>
/// Dispose Direct2d resources 
/// </summary>
void CBodyBasics::DiscardDirect2DResources()
{
    SafeRelease(m_pRenderTarget);

    SafeRelease(m_pBrushJointTracked);
    SafeRelease(m_pBrushJointInferred);
    SafeRelease(m_pBrushBoneTracked);
    SafeRelease(m_pBrushBoneInferred);

    SafeRelease(m_pBrushHandClosed);
    SafeRelease(m_pBrushHandOpen);
    SafeRelease(m_pBrushHandLasso);
}

/// <summary>
/// Converts a body point to screen space
/// </summary>
/// <param name="bodyPoint">body point to tranform</param>
/// <param name="width">width (in pixels) of output buffer</param>
/// <param name="height">height (in pixels) of output buffer</param>
/// <returns>point in screen-space</returns>
D2D1_POINT_2F CBodyBasics::BodyToScreen(const CameraSpacePoint& bodyPoint, int width, int height)
{
    // Calculate the body's position on the screen
    DepthSpacePoint depthPoint = {0};
    m_pCoordinateMapper->MapCameraPointToDepthSpace(bodyPoint, &depthPoint);

    float screenPointX = static_cast<float>(depthPoint.X * width) / cDepthWidth;
    float screenPointY = static_cast<float>(depthPoint.Y * height) / cDepthHeight;

    return D2D1::Point2F(screenPointX, screenPointY);
}

/// <summary>
/// Draws a body 
/// </summary>
/// <param name="pJoints">joint data</param>
/// <param name="pJointPoints">joint positions converted to screen space</param>
void CBodyBasics::DrawBody(const Joint* pJoints, const D2D1_POINT_2F* pJointPoints)
{
    // Draw the bones

    // Torso
    DrawBone(pJoints, pJointPoints, JointType_Head, JointType_Neck);
    DrawBone(pJoints, pJointPoints, JointType_Neck, JointType_SpineShoulder);
    DrawBone(pJoints, pJointPoints, JointType_SpineShoulder, JointType_SpineMid);
    DrawBone(pJoints, pJointPoints, JointType_SpineMid, JointType_SpineBase);
    DrawBone(pJoints, pJointPoints, JointType_SpineShoulder, JointType_ShoulderRight);
    DrawBone(pJoints, pJointPoints, JointType_SpineShoulder, JointType_ShoulderLeft);
    DrawBone(pJoints, pJointPoints, JointType_SpineBase, JointType_HipRight);
    DrawBone(pJoints, pJointPoints, JointType_SpineBase, JointType_HipLeft);
    
    // Right Arm    
    DrawBone(pJoints, pJointPoints, JointType_ShoulderRight, JointType_ElbowRight);
    DrawBone(pJoints, pJointPoints, JointType_ElbowRight, JointType_WristRight);
    DrawBone(pJoints, pJointPoints, JointType_WristRight, JointType_HandRight);
    DrawBone(pJoints, pJointPoints, JointType_HandRight, JointType_HandTipRight);
    DrawBone(pJoints, pJointPoints, JointType_WristRight, JointType_ThumbRight);

    // Left Arm
    DrawBone(pJoints, pJointPoints, JointType_ShoulderLeft, JointType_ElbowLeft);
    DrawBone(pJoints, pJointPoints, JointType_ElbowLeft, JointType_WristLeft);
    DrawBone(pJoints, pJointPoints, JointType_WristLeft, JointType_HandLeft);
    DrawBone(pJoints, pJointPoints, JointType_HandLeft, JointType_HandTipLeft);
    DrawBone(pJoints, pJointPoints, JointType_WristLeft, JointType_ThumbLeft);

    // Right Leg
    DrawBone(pJoints, pJointPoints, JointType_HipRight, JointType_KneeRight);
    DrawBone(pJoints, pJointPoints, JointType_KneeRight, JointType_AnkleRight);
    DrawBone(pJoints, pJointPoints, JointType_AnkleRight, JointType_FootRight);

    // Left Leg
    DrawBone(pJoints, pJointPoints, JointType_HipLeft, JointType_KneeLeft);
    DrawBone(pJoints, pJointPoints, JointType_KneeLeft, JointType_AnkleLeft);
    DrawBone(pJoints, pJointPoints, JointType_AnkleLeft, JointType_FootLeft);

    // Draw the joints
    for (int i = 0; i < JointType_Count; ++i)
    {
        D2D1_ELLIPSE ellipse = D2D1::Ellipse(pJointPoints[i], c_JointThickness, c_JointThickness);

        if (pJoints[i].TrackingState == TrackingState_Inferred)
        {
            m_pRenderTarget->FillEllipse(ellipse, m_pBrushJointInferred);
        }
        else if (pJoints[i].TrackingState == TrackingState_Tracked)
        {
            m_pRenderTarget->FillEllipse(ellipse, m_pBrushJointTracked);
        }
    }
}

/// <summary>
/// Draws one bone of a body (joint to joint)
/// </summary>
/// <param name="pJoints">joint data</param>
/// <param name="pJointPoints">joint positions converted to screen space</param>
/// <param name="pJointPoints">joint positions converted to screen space</param>
/// <param name="joint0">one joint of the bone to draw</param>
/// <param name="joint1">other joint of the bone to draw</param>
void CBodyBasics::DrawBone(const Joint* pJoints, const D2D1_POINT_2F* pJointPoints, JointType joint0, JointType joint1)
{
    TrackingState joint0State = pJoints[joint0].TrackingState;
    TrackingState joint1State = pJoints[joint1].TrackingState;

    // If we can't find either of these joints, exit
    if ((joint0State == TrackingState_NotTracked) || (joint1State == TrackingState_NotTracked))
    {
        return;
    }

    // Don't draw if both points are inferred
    if ((joint0State == TrackingState_Inferred) && (joint1State == TrackingState_Inferred))
    {
        return;
    }

    // We assume all drawn bones are inferred unless BOTH joints are tracked
    if ((joint0State == TrackingState_Tracked) && (joint1State == TrackingState_Tracked))
    {
        m_pRenderTarget->DrawLine(pJointPoints[joint0], pJointPoints[joint1], m_pBrushBoneTracked, c_TrackedBoneThickness);
    }
    else
    {
        m_pRenderTarget->DrawLine(pJointPoints[joint0], pJointPoints[joint1], m_pBrushBoneInferred, c_InferredBoneThickness);
    }
}

/// <summary>
/// Draws a hand symbol if the hand is tracked: red circle = closed, green circle = opened; blue circle = lasso
/// </summary>
/// <param name="handState">state of the hand</param>
/// <param name="handPosition">position of the hand</param>
void CBodyBasics::DrawHand(HandState handState, const D2D1_POINT_2F& handPosition)
{
    D2D1_ELLIPSE ellipse = D2D1::Ellipse(handPosition, c_HandSize, c_HandSize);

    switch (handState)
    {
        case HandState_Closed:
            m_pRenderTarget->FillEllipse(ellipse, m_pBrushHandClosed);
            break;

        case HandState_Open:
            m_pRenderTarget->FillEllipse(ellipse, m_pBrushHandOpen);
            break;

        case HandState_Lasso:
            m_pRenderTarget->FillEllipse(ellipse, m_pBrushHandLasso);
            break;
    }
}

void CBodyBasics::UpdateEnemy()
{
	int enemy_size_x = 60;
	int enemy_size_y = 60;

	size_t count = 0;
	for (size_t i = 0; i < m_enemys_vec.size(); i++)
	{
		bool visible = m_enemys_vec[i]->move(m_game_view_height);

		if (visible)
		{
			m_enemys_vec[count] = m_enemys_vec[i];
			count++;
		}
		else
		{
			m_enemy_pass += 1;
			m_redline_counter = 30;
			delete m_enemys_vec[i];
		}
	}
	m_enemys_vec.resize(count);

	if (m_enemy_count == m_enemy_next_count)
	{
		// Generate random x coordinate for enemy
		int x_start = rand() % (m_game_view_width - enemy_size_x);

		m_enemys_vec.push_back(new  Enemy(
			x_start,
			0,
			enemy_size_x, enemy_size_y));

		m_enemy_next_count = rand() % 50 + 10;
		m_enemy_count = 0;
	}
	else
	{
		m_enemy_count++;
	}
}

void CBodyBasics::UpdateBullet()
{
	int bullet_size_x = 30;
	int bullet_size_y = 30;

	size_t count = 0;
	for (size_t i = 0; i < m_bullets_vec.size(); i++)
	{
		bool visible = m_bullets_vec[i]->move();

		if (visible)
		{
			m_bullets_vec[count] = m_bullets_vec[i];
			count++;
		}
		else
		{
			delete m_bullets_vec[i];
		}
	}
	m_bullets_vec.resize(count);

	if (m_bullet_count == 10)
	{
		m_bullets_vec.push_back(new  Bullet(
			m_pPlane->getX() + m_pPlane->getWidth() / 3,
			m_game_view_height - m_plane_size_height - bullet_size_y,
			bullet_size_x, bullet_size_y));
		m_bullets_vec[m_bullets_vec.size() - 1]->setVisible(true);

		m_bullet_count = 0;
	}
	else
	{
		m_bullet_count++;
	}
	
}

void CBodyBasics::CollisionDetection()
{
	CollisionDetector detector;
	int enemy_count = 0;
	bool shooted;
	for (size_t j = 0; j < m_enemys_vec.size(); j++)
	{
		shooted = false;
		for (size_t i = 0; i < m_bullets_vec.size(); i++)
		{
			bool isCollision = detector.check(*(m_bullets_vec[i]), *(m_enemys_vec[j]));
			if (isCollision)
			{
				m_bullets_vec[i]->setVisible(false);
				delete m_enemys_vec[j];
				shooted = true;
				m_score++;
				break;
			}
		}

		if (!shooted)
		{
			m_enemys_vec[enemy_count] = m_enemys_vec[j];
			enemy_count++;
		}
	}
	m_enemys_vec.resize(enemy_count);
}


/// <summary>
/// Draws a hand symbol if the hand is tracked: red circle = closed, green circle = opened; blue circle = lasso
/// </summary>
/// <param name="handState">state of the hand</param>
/// <param name="handPosition">position of the hand</param>
double CBodyBasics::DrawGameViewActive(const Joint* pJoints, const D2D1_POINT_2F* pJointPoints, int width, int height)
{
	// Initialize 3 2 1 go
	if (!m_isTracking)
	{
		m_isTracking = true;
		m_isGameStart = false;
		// Game start

		m_init_counter = 90;

		m_enemy_pass = 0;
		m_score = 0;

		// Delete all flying enemy plane
		for (size_t i = 0; i < m_enemys_vec.size(); i++)
		{
			delete m_enemys_vec[i];
		}
		m_enemys_vec.resize(0);
	}

	m_pRenderGame->BeginDraw();
	m_pRenderGame->Clear();

	m_pDraw->drawNextBackground(width, height);

	if (m_isGameOver)
	{
		if (m_init_counter)
		{
			m_pDraw->drawGameOver(width, height);
		}

		if (m_init_counter == 0)
		{
			m_isGameOver = false;
			m_isGameStart = false;
			m_isTracking = false;
			m_enemy_pass = 0;
			
			// Delete all flying enemy plane
			for (size_t i = 0; i < m_enemys_vec.size(); i++)
			{
				delete m_enemys_vec[i];
			}
			m_enemys_vec.resize(0);

			// Delete all flying enemy plane
			for (size_t i = 0; i < m_bullets_vec.size(); i++)
			{
				delete m_bullets_vec[i];
			}
			m_bullets_vec.resize(0);
		}
		m_init_counter--;

		float arm_slope;
		bool res = GetArmSlope(pJoints, pJointPoints, arm_slope);

		m_pPlane->move(arm_slope);
		m_pPlane->render(m_pDraw);

		HRESULT hr = m_pRenderGame->EndDraw();

		// Device lost, need to recreate the render target
		// We'll dispose it now and retry drawing
		if (D2DERR_RECREATE_TARGET == hr)
		{
			hr = S_OK;
			DiscardDirect2DResources();
		}

		return 0;
	}

	// 3, 2, 1, Go display
	// GameStart = false
	if (!m_isGameStart)
	{
		if (m_init_counter > 80)
		{
			m_pDraw->drawThree(width, height);
		}
		else if (m_init_counter < 70 && m_init_counter > 50)
		{
			m_pDraw->drawTwo(width, height);
		}
		else if (m_init_counter < 45 && m_init_counter > 25)
		{
			m_pDraw->drawOne(width, height);
		}
		else if (m_init_counter < 20)
		{
			m_pDraw->drawGo(width, height);
		}
		
		if (m_init_counter == 0)
		{
			m_isGameStart = true;
		}
		m_init_counter--;
		
		float arm_slope;
		bool res = GetArmSlope(pJoints, pJointPoints, arm_slope);

		m_pPlane->move(arm_slope);
		m_pPlane->render(m_pDraw);

		HRESULT hr = m_pRenderGame->EndDraw();

		// Device lost, need to recreate the render target
		// We'll dispose it now and retry drawing
		if (D2DERR_RECREATE_TARGET == hr)
		{
			hr = S_OK;
			DiscardDirect2DResources();
		}

		return 0;
	}

	float arm_slope;
	bool res = GetArmSlope(pJoints, pJointPoints, arm_slope);


	UpdateEnemy();
	UpdateBullet();
	CollisionDetection();

	for (size_t i = 0; i < m_enemys_vec.size(); i++)
	{
		m_enemys_vec[i]->render(m_pDraw);
	}

	for (size_t i = 0; i < m_bullets_vec.size(); i++)
	{
		if (m_bullets_vec[i]->getVisible())
		{
			m_bullets_vec[i]->render(m_pDraw);
		}
	}

	if (m_redline_counter)
	{
		m_redline_counter--;

		if (m_redline_counter % 6 < 3)
		{
			m_pDraw->drawRedline(0, m_game_view_height - 12, m_game_view_width, 12);
		}
	}
	

	m_pPlane->move(arm_slope);
	m_pPlane->render(m_pDraw);

	if (m_enemy_pass == 3)
	{
		m_isGameOver = true;
		//m_isGameStart = true;

		m_init_counter = 140;
	}

	D2D1_RECT_F layoutRect = D2D1::RectF(0, 0, 200.f, 200.f);
	WCHAR str[100];
	swprintf_s(str, 100, L"Score: %d", m_score);
	//draw text  
	m_pRenderTarget->DrawText(
		str,
		wcslen(str),
		m_pTextFormat,
		layoutRect,
		m_pBrushScore
	);


	HRESULT hr = m_pRenderGame->EndDraw();

	// Device lost, need to recreate the render target
	// We'll dispose it now and retry drawing
	if (D2DERR_RECREATE_TARGET == hr)
	{
		hr = S_OK;
		DiscardDirect2DResources();
	}

	return arm_slope;
}

void CBodyBasics::DrawGameViewInactive(int width, int height)
{
	m_isTracking = false;
	m_isGameStart = false;
	m_isGameOver = false;
	m_score = 0;
	m_enemy_pass = 0;

	m_pRenderGame->BeginDraw();
	m_pRenderGame->Clear();



	m_pDraw->drawNextBackground(width, height);
	m_pDraw->drawMyPlane(width / 2 - m_plane_size_width, height - m_plane_size_height, 80, 80);

	HRESULT hr = m_pRenderGame->EndDraw();

	// Device lost, need to recreate the render target
	// We'll dispose it now and retry drawing
	if (D2DERR_RECREATE_TARGET == hr)
	{
		hr = S_OK;
		DiscardDirect2DResources();
	}
}


bool CBodyBasics::GetArmSlope(const Joint* pJoints, const D2D1_POINT_2F* pJointPoints, float& degree)
{
	JointType joint_type;
	degree = 0;

	std::vector<int> joint_id;
	joint_id.push_back(JointType_ThumbLeft);
	joint_id.push_back(JointType_HandTipLeft);
	joint_id.push_back(JointType_HandLeft);
	joint_id.push_back(JointType_WristLeft);
	joint_id.push_back(JointType_ElbowLeft);
	joint_id.push_back(JointType_ShoulderLeft);
	joint_id.push_back(JointType_ElbowRight);
	joint_id.push_back(JointType_WristRight);
	joint_id.push_back(JointType_HandRight);
	joint_id.push_back(JointType_HandTipRight);
	joint_id.push_back(JointType_ThumbRight);

	std::vector<TrackingState> joint_state(joint_id.size());
	for (size_t i = 0; i < joint_state.size(); i++)
	{
		joint_state[i] = pJoints[i].TrackingState;
	}

	size_t left_hand_id = 2;
	size_t right_hand_id = joint_state.size() - 3;

	// If we can't find either of these joints, exit
	if ((joint_state[left_hand_id] == TrackingState_NotTracked) || (joint_state[right_hand_id] == TrackingState_NotTracked))
	{
		return false;
	}

	// Don't draw if both points are inferred
	if ((joint_state[left_hand_id] == TrackingState_Inferred) && (joint_state[right_hand_id] == TrackingState_Inferred))
	{
		return false;
	}

	/*slope = 
		float(pJointPoints[JointType_HandLeft].y - pJointPoints[JointType_HandRight].y) /
		float(pJointPoints[JointType_HandLeft].x - pJointPoints[JointType_HandRight].x);*/

	degree = atan2(float(pJointPoints[JointType_HandLeft].y - pJointPoints[JointType_HandRight].y), float(pJointPoints[JointType_HandLeft].x - pJointPoints[JointType_HandRight].x)) / 3.1415926 * 180;

	if (degree > 0)
	{
		degree = -180 + degree;
	}
	else
	{
		degree = 180 + degree;
	}

	//// We assume all drawn bones are inferred unless BOTH joints are tracked
	//if ((joint0State == TrackingState_Tracked) && (joint1State == TrackingState_Tracked))
	//{
	//	m_pRenderTarget->DrawLine(pJointPoints[joint0], pJointPoints[joint1], m_pBrushBoneTracked, c_TrackedBoneThickness);
	//}
	//else
	//{
	//	m_pRenderTarget->DrawLine(pJointPoints[joint0], pJointPoints[joint1], m_pBrushBoneInferred, c_InferredBoneThickness);
	//}

	return true;
}