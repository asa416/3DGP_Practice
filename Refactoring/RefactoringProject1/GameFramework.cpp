#include "pch.h"
#include "GameFramework.h"

GameFramework::GameFramework() :
	m_hInstance{},
	m_hWnd{},
	m_nWndClientWidth{},
	m_nWndClientHeight{},
	m_Factory{},
	m_SwapChain{},
	m_Device{},
	m_nSwapChainBufferIndex{ 0 },
	//m_ppd3dRenderTargetBuffers{},
	//m_pd3dRtvDescriptorHeap{ nullptr },
	m_RenderTargetBuffers{},
	m_RtvDescriptorHeap{},
	m_nRtvDescriptorIncrementSize{ 0 },
	//m_pd3dDepthStencilBuffer{ nullptr },
	//m_pd3dDsvDescriptorHeap{ nullptr },
	m_DepthStencilBuffer{},
	m_DsvDescriptorHeap{},
	m_nDsvDescriptorIncrementSize{ 0 },
	//m_pd3dCommandQueue{ nullptr },
	//m_pd3dCommandAllocator{ nullptr },
	//m_pd3dCommandList{ nullptr },
	m_CommandQueue{},
	m_CommandAllocator{},
	m_CommandList{},
	//m_pd3dPipelineState{ nullptr },
	m_PipelineState{},
	//m_pd3dFence{ nullptr },
	m_Fence{},
	m_nFenceValue{ 0 },
	m_hFenceEvent{nullptr},
	m_d3dViewport{},
	m_d3dScissorRect{}
{
}

GameFramework::~GameFramework()
{
}

bool GameFramework::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	m_hInstance = hInstance;
	m_hWnd = hMainWnd;

	CreateDirect3DDevice();
	CreateCommandQueueAndList();
	CreateSwapChain();
	CreateRtvAndDsvDescriptorHeaps();
	CreateRenderTargetViews();
	CreateDepthStencilView();

	BuildObjects();

	return true;
}

void GameFramework::OnDestory()
{
	WaitForGpuComplete();

	ReleaseObjects();

	::CloseHandle(m_hFenceEvent);

	/*for (int i = 0; i < m_nSwapChainBuffers; ++i) if (m_ppd3dRenderTargetBuffers[i]) m_ppd3dRenderTargetBuffers[i]->Release();
	if (m_pd3dRtvDescriptorHeap) m_pd3dRtvDescriptorHeap->Release();
	if (m_pd3dDepthStencilBuffer) m_pd3dDepthStencilBuffer->Release();
	if (m_pd3dDsvDescriptorHeap) m_pd3dDsvDescriptorHeap->Release();
	if (m_pd3dCommandAllocator) m_pd3dCommandAllocator->Release();
	if (m_pd3dCommandQueue) m_pd3dCommandQueue->Release();
	if (m_pd3dPipelineState) m_pd3dPipelineState->Release();
	if (m_pd3dCommandList) m_pd3dCommandList->Release();
	if (m_pd3dFence) m_pd3dFence->Release();
	m_pdxgiSwapChain->SetFullscreenState(FALSE, NULL);
	if (m_pdxgiSwapChain) m_pdxgiSwapChain->Release();
	if (m_pd3dDevice) m_pd3dDevice->Release();
	if (m_pdxgiFactory) m_pdxgiFactory->Release();*/
}

void GameFramework::CreateSwapChain()
{
	RECT rcClient;
	::GetClientRect(m_hWnd, &rcClient);
	m_nWndClientWidth = rcClient.right - rcClient.left;
	m_nWndClientHeight = rcClient.bottom - rcClient.top;
	DXGI_SWAP_CHAIN_DESC1 dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC1));
	dxgiSwapChainDesc.Width = m_nWndClientWidth;
	dxgiSwapChainDesc.Height = m_nWndClientHeight;
	dxgiSwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.BufferCount = m_nSwapChainBuffers;
	dxgiSwapChainDesc.Scaling = DXGI_SCALING_NONE;
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgiSwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	dxgiSwapChainDesc.Flags = 0;
	DXGI_SWAP_CHAIN_FULLSCREEN_DESC dxgiSwapChainFullScreenDesc;
	::ZeroMemory(&dxgiSwapChainFullScreenDesc, sizeof(DXGI_SWAP_CHAIN_FULLSCREEN_DESC));
	dxgiSwapChainFullScreenDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainFullScreenDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainFullScreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Windowed = TRUE;
	/*m_pdxgiFactory->CreateSwapChainForHwnd(m_pd3dCommandQueue, m_hWnd,
		&dxgiSwapChainDesc, &dxgiSwapChainFullScreenDesc, NULL, (IDXGISwapChain1
			**)&m_pdxgiSwapChain);*/
	ComPtr<IDXGISwapChain1> SwapChain;
	m_Factory->CreateSwapChainForHwnd(m_CommandQueue.Get(), m_hWnd, &dxgiSwapChainDesc, &dxgiSwapChainFullScreenDesc, nullptr, &SwapChain);
	//����ü���� �����Ѵ�.
	//m_pdxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);
	m_Factory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);
	//��Alt+Enter�� Ű�� ������ ��Ȱ��ȭ�Ѵ�.
	SwapChain.As(&m_SwapChain);
	m_nSwapChainBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();
	//����ü���� ���� �ĸ���� �ε����� �����Ѵ�.
}

void GameFramework::CreateRtvAndDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = m_nSwapChainBuffers;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = m_Device->CreateDescriptorHeap(&d3dDescriptorHeapDesc,
		__uuidof(ID3D12DescriptorHeap), (void**)&m_RtvDescriptorHeap);
	//���� Ÿ�� ������ ��(�������� ������ ����ü�� ������ ����)�� �����Ѵ�.
	m_nRtvDescriptorIncrementSize =
		m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	//���� Ÿ�� ������ ���� ������ ũ�⸦ �����Ѵ�.
	d3dDescriptorHeapDesc.NumDescriptors = 1;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	hResult = m_Device->CreateDescriptorHeap(&d3dDescriptorHeapDesc,
		__uuidof(ID3D12DescriptorHeap), (void**)&m_DsvDescriptorHeap);
	//����-���ٽ� ������ ��(�������� ������ 1)�� �����Ѵ�.
	m_nDsvDescriptorIncrementSize =
		m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	//����-���ٽ� ������ ���� ������ ũ�⸦ �����Ѵ�.
}

void GameFramework::CreateDirect3DDevice()
{
	HRESULT hResult;
	UINT nDXGIFactoryFlags = 0;
#if defined(_DEBUG)
	ID3D12Debug* pd3dDebugController = NULL;
	hResult = D3D12GetDebugInterface(__uuidof(ID3D12Debug), (void
		**)&pd3dDebugController);
	if (pd3dDebugController)
	{
		pd3dDebugController->EnableDebugLayer();
		pd3dDebugController->Release();
	}
	nDXGIFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
	hResult = ::CreateDXGIFactory2(nDXGIFactoryFlags, __uuidof(IDXGIFactory4), (void
		**)&m_Factory);
	IDXGIAdapter1* pd3dAdapter = NULL;
	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != m_Factory->EnumAdapters1(i,
		&pd3dAdapter); i++)
	{
		DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
		pd3dAdapter->GetDesc1(&dxgiAdapterDesc);
		if (dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
		if (SUCCEEDED(D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_12_0,
			_uuidof(ID3D12Device), (void**)&m_Device))) break;
	}
	//��� �ϵ���� ����� ���Ͽ� Ư�� ���� 12.0�� �����ϴ� �ϵ���� ����̽��� �����Ѵ�.
	if (!pd3dAdapter)
	{
		m_Factory->EnumWarpAdapter(_uuidof(IDXGIAdapter1), (void**)&pd3dAdapter);
		D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), (void
			**)&m_Device);
	}
	//Ư�� ���� 12.0�� �����ϴ� �ϵ���� ����̽��� ������ �� ������ WARP ����̽��� �����Ѵ�.
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS d3dMsaaQualityLevels;
	d3dMsaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dMsaaQualityLevels.SampleCount = 4; //Msaa4x ���� ���ø�
	d3dMsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	d3dMsaaQualityLevels.NumQualityLevels = 0;
	m_Device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&d3dMsaaQualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
	m_nMsaa4xQualityLevels = d3dMsaaQualityLevels.NumQualityLevels;
	//����̽��� �����ϴ� ���� ������ ǰ�� ������ Ȯ���Ѵ�.
	m_bMsaa4xEnable = (m_nMsaa4xQualityLevels > 1) ? true : false;
	//���� ������ ǰ�� ������ 1���� ũ�� ���� ���ø��� Ȱ��ȭ�Ѵ�.
	hResult = m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence),
		(void**)&m_Fence);
	m_nFenceValue = 0;

	//�潺�� �����ϰ� �潺 ���� 0���� �����Ѵ�.
	m_hFenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	/*�潺�� ����ȭ�� ���� �̺�Ʈ ��ü�� �����Ѵ�(�̺�Ʈ ��ü�� �ʱⰪ�� FALSE�̴�). �̺�Ʈ�� ����Ǹ�(Signal) ��
	��Ʈ�� ���� �ڵ������� FALSE�� �ǵ��� �����Ѵ�.*/
	m_d3dViewport.TopLeftX = 0;
	m_d3dViewport.TopLeftY = 0;
	m_d3dViewport.Width = static_cast<float>(m_nWndClientWidth);
	m_d3dViewport.Height = static_cast<float>(m_nWndClientHeight);
	m_d3dViewport.MinDepth = 0.0f;
	m_d3dViewport.MaxDepth = 1.0f;
	//����Ʈ�� �� �������� Ŭ���̾�Ʈ ���� ��ü�� �����Ѵ�.
	m_d3dScissorRect = { 0, 0, m_nWndClientWidth, m_nWndClientHeight };
	//���� �簢���� �� �������� Ŭ���̾�Ʈ ���� ��ü�� �����Ѵ�.
	if (pd3dAdapter) pd3dAdapter->Release();
}

void GameFramework::CreateCommandQueueAndList()
{
	D3D12_COMMAND_QUEUE_DESC d3dCommandQueueDesc;
	::ZeroMemory(&d3dCommandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));
	d3dCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	d3dCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	HRESULT hResult = m_Device->CreateCommandQueue(&d3dCommandQueueDesc,
		_uuidof(ID3D12CommandQueue), (void**)&m_CommandQueue);
	//����(Direct) ���� ť�� �����Ѵ�.
	hResult = m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		__uuidof(ID3D12CommandAllocator), (void**)&m_CommandAllocator);
	//����(Direct) ���� �Ҵ��ڸ� �����Ѵ�.
	hResult = m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_CommandAllocator.Get(), NULL, __uuidof(ID3D12GraphicsCommandList), (void
			**)&m_CommandList);
	//����(Direct) ���� ����Ʈ�� �����Ѵ�.
	hResult = m_CommandList->Close();
	//���� ����Ʈ�� �����Ǹ� ����(Open) �����̹Ƿ� ����(Closed) ���·� �����.
}

void GameFramework::CreateRenderTargetViews()
{
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle =
		m_RtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < m_nSwapChainBuffers; i++)
	{
		m_SwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void
			**)&m_RenderTargetBuffers[i]);
		m_Device->CreateRenderTargetView(m_RenderTargetBuffers[i].Get(), NULL,
			d3dRtvCPUDescriptorHandle);
		d3dRtvCPUDescriptorHandle.ptr += m_nRtvDescriptorIncrementSize;
	}
}

void GameFramework::CreateDepthStencilView()
{
	D3D12_RESOURCE_DESC d3dResourceDesc;
	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	d3dResourceDesc.Alignment = 0;
	d3dResourceDesc.Width = m_nWndClientWidth;
	d3dResourceDesc.Height = m_nWndClientHeight;
	d3dResourceDesc.DepthOrArraySize = 1;
	d3dResourceDesc.MipLevels = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dResourceDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	d3dResourceDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1)
		: 0;
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	D3D12_HEAP_PROPERTIES d3dHeapProperties;
	::ZeroMemory(&d3dHeapProperties, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapProperties.CreationNodeMask = 1;
	d3dHeapProperties.VisibleNodeMask = 1;
	D3D12_CLEAR_VALUE d3dClearValue;
	d3dClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dClearValue.DepthStencil.Depth = 1.0f;
	d3dClearValue.DepthStencil.Stencil = 0;
	m_Device->CreateCommittedResource(&d3dHeapProperties, D3D12_HEAP_FLAG_NONE,
		&d3dResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClearValue,
		__uuidof(ID3D12Resource), (void**)&m_DepthStencilBuffer);
	//����-���ٽ� ���۸� �����Ѵ�.
	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle =
		m_DsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_Device->CreateDepthStencilView(m_DepthStencilBuffer.Get(), NULL,
		d3dDsvCPUDescriptorHandle);
	//����-���ٽ� ���� �並 �����Ѵ�.
}

void GameFramework::BuildObjects()
{
}

void GameFramework::ReleaseObjects()
{
}

void GameFramework::ProcessInput()
{
}

void GameFramework::AnimateObjects()
{
}

void GameFramework::FrameAdvance()
{
	ProcessInput();
	AnimateObjects();
	HRESULT hResult = m_CommandAllocator->Reset();
	hResult = m_CommandList->Reset(m_CommandAllocator.Get(), NULL);
	//���� �Ҵ��ڿ� ���� ����Ʈ�� �����Ѵ�.
	D3D12_RESOURCE_BARRIER d3dResourceBarrier;
	::ZeroMemory(&d3dResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
	d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	d3dResourceBarrier.Transition.pResource =
		m_RenderTargetBuffers[m_nSwapChainBufferIndex].Get();
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_CommandList->ResourceBarrier(1, &d3dResourceBarrier);
	/*���� ���� Ÿ�ٿ� ���� ������Ʈ�� �����⸦ ��ٸ���. ������Ʈ�� ������ ���� Ÿ�� ������ ���´� ������Ʈ ����
	(D3D12_RESOURCE_STATE_PRESENT)���� ���� Ÿ�� ����(D3D12_RESOURCE_STATE_RENDER_TARGET)�� ��
	�� ���̴�.*/
	m_CommandList->RSSetViewports(1, &m_d3dViewport);
	m_CommandList->RSSetScissorRects(1, &m_d3dScissorRect);
	//����Ʈ�� ���� �簢���� �����Ѵ�.
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle =
		m_RtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += (m_nSwapChainBufferIndex *
		m_nRtvDescriptorIncrementSize);
	float pfClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	m_CommandList->ClearRenderTargetView(d3dRtvCPUDescriptorHandle,
		pfClearColor/*Colors::Azure*/, 0, NULL);
	//���ϴ� �������� ���� Ÿ��(��)�� �����.
	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle =
		m_DsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	//����-���ٽ� �������� CPU �ּҸ� ����Ѵ�.
	m_CommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle,
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
	//���ϴ� ������ ����-���ٽ�(��)�� �����.
	m_CommandList->OMSetRenderTargets(1, &d3dRtvCPUDescriptorHandle, TRUE,
		&d3dDsvCPUDescriptorHandle);
	//���� Ÿ�� ��(������)�� ����-���ٽ� ��(������)�� ���-���� �ܰ�(OM)�� �����Ѵ�.
	//������ �ڵ�� ���⿡ �߰��� ���̴�.
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_CommandList->ResourceBarrier(1, &d3dResourceBarrier);
	/*���� ���� Ÿ�ٿ� ���� �������� �����⸦ ��ٸ���. GPU�� ���� Ÿ��(����)�� �� �̻� ������� ������ ���� Ÿ��
	�� ���´� ������Ʈ ����(D3D12_RESOURCE_STATE_PRESENT)�� �ٲ� ���̴�.*/
	hResult = m_CommandList->Close();
	//���� ����Ʈ�� ���� ���·� �����.
	ID3D12CommandList* ppd3dCommandLists[] = { m_CommandList.Get()};
	m_CommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);
	//���� ����Ʈ�� ���� ť�� �߰��Ͽ� �����Ѵ�.
	WaitForGpuComplete();
	//GPU�� ��� ���� ����Ʈ�� ������ �� ���� ��ٸ���.
	DXGI_PRESENT_PARAMETERS dxgiPresentParameters;
	dxgiPresentParameters.DirtyRectsCount = 0;
	dxgiPresentParameters.pDirtyRects = NULL;
	dxgiPresentParameters.pScrollRect = NULL;
	dxgiPresentParameters.pScrollOffset = NULL;
	m_SwapChain->Present1(1, 0, &dxgiPresentParameters);
	/*����ü���� ������Ʈ�Ѵ�. ������Ʈ�� �ϸ� ���� ���� Ÿ��(�ĸ����)�� ������ ������۷� �Ű����� ���� Ÿ�� ��
	������ �ٲ� ���̴�.*/
	m_nSwapChainBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();
}

void GameFramework::WaitForGpuComplete()
{
	m_nFenceValue++;
	//CPU �潺�� ���� �����Ѵ�.
	const UINT64 nFence = m_nFenceValue;
	HRESULT hResult = m_CommandQueue->Signal(m_Fence.Get(), nFence);
	//GPU�� �潺�� ���� �����ϴ� ������ ���� ť�� �߰��Ѵ�.
	if (m_Fence->GetCompletedValue() < nFence)
	{
		//�潺�� ���� ���� ������ ������ ������ �潺�� ���� ���� ������ ���� �� ������ ��ٸ���.
		hResult = m_Fence->SetEventOnCompletion(nFence, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}

void GameFramework::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		break;
	case WM_MOUSEMOVE:
		break;
	default:
		break;
	}
}

void GameFramework::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYUP:
		switch (wParam)
		{
		case VK_ESCAPE:
			::PostQuitMessage(0);
			break;
		case VK_RETURN:
			break;
		case VK_F8:
			break;
		case VK_F9:
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

LRESULT GameFramework::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_SIZE:
	{
		m_nWndClientWidth = LOWORD(lParam);
		m_nWndClientHeight = HIWORD(lParam);
		break;
	}
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
		OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
		OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
		break;
	}
	return 0;
}