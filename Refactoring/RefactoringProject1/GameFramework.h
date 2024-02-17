#pragma once
class GameFramework
{
private:
	HINSTANCE m_hInstance;
	HWND m_hWnd;

	int m_nWndClientWidth;
	int m_nWndClientHeight;

	//IDXGIFactory4* m_pdxgiFactory;
	//IDXGISwapChain3* m_pdxgiSwapChain;
	//ID3D12Device* m_pd3dDevice;
	ComPtr<IDXGIFactory4> m_Factory;
	ComPtr<IDXGISwapChain3> m_SwapChain;
	ComPtr<ID3D12Device> m_Device;

	bool m_bMsaa4xEnable{ false };
	UINT m_nMsaa4xQualityLevels{};

	static constexpr UINT m_nSwapChainBuffers{ 2 };
	UINT m_nSwapChainBufferIndex;

	//ID3D12Resource* m_ppd3dRenderTargetBuffers[m_nSwapChainBuffers];
	//ID3D12DescriptorHeap* m_pd3dRtvDescriptorHeap;
	ComPtr<ID3D12Resource> m_RenderTargetBuffers[m_nSwapChainBuffers];
	ComPtr<ID3D12DescriptorHeap> m_RtvDescriptorHeap;
	UINT m_nRtvDescriptorIncrementSize;

	//ID3D12Resource* m_pd3dDepthStencilBuffer;
	//ID3D12DescriptorHeap* m_pd3dDsvDescriptorHeap;
	ComPtr<ID3D12Resource> m_DepthStencilBuffer;
	ComPtr<ID3D12DescriptorHeap> m_DsvDescriptorHeap;
	UINT m_nDsvDescriptorIncrementSize;

	//ID3D12CommandQueue* m_pd3dCommandQueue;
	//ID3D12CommandAllocator* m_pd3dCommandAllocator;
	//ID3D12GraphicsCommandList* m_pd3dCommandList;
	ComPtr<ID3D12CommandQueue> m_CommandQueue;
	ComPtr<ID3D12CommandAllocator> m_CommandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_CommandList;

	//ID3D12PipelineState* m_pd3dPipelineState;
	ComPtr<ID3D12PipelineState> m_PipelineState;

	//ID3D12Fence* m_pd3dFence;
	ComPtr<ID3D12Fence> m_Fence;
	UINT64 m_nFenceValue;
	HANDLE m_hFenceEvent;

	D3D12_VIEWPORT m_d3dViewport;
	D3D12_RECT m_d3dScissorRect;

public:
	GameFramework();
	~GameFramework();

	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	void OnDestory();

	void CreateSwapChain();
	void CreateRtvAndDsvDescriptorHeaps();
	void CreateDirect3DDevice();
	void CreateCommandQueueAndList();

	void CreateRenderTargetViews();
	void CreateDepthStencilView();
	
	void BuildObjects();
	void ReleaseObjects();

	void ProcessInput();
	void AnimateObjects();
	void FrameAdvance();

	void WaitForGpuComplete();

	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
};

