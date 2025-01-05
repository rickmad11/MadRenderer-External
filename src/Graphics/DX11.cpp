#include "pch.h"
#include "DX11.h"

#include "../Window/WindowManager.h"
#include "CommonStates.h"
#include "font.h"

DX11* DX11::Get()
{
	static DX11 instance;
	return &instance;
}

void DX11::OnWindowResize(UINT width, UINT height) noexcept
{
	DX11* p_dx11 = Get();

	if(p_dx11->pSwapChain)
	{
		p_dx11->isBeingResized = true;

		width = max(width, 1);
		height = max(height, 1);

		(void)p_dx11->pRenderTargetView.Reset();

		BOOL bFullscreenState = false;

		p_dx11->pSwapChain->GetFullscreenState(&bFullscreenState, nullptr);

		if(bFullscreenState)
			p_dx11->pSwapChain->SetFullscreenState(false, nullptr);

		if (FAILED(p_dx11->pSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0)))
		{
			OutputDebugStringW(L"\n\n\nError on ResizeBuffers\n\n\n");
			p_dx11->isBeingResized = false;
			return;
		}

		Microsoft::WRL::ComPtr<ID3D11Texture2D> p_texture_2d = nullptr;
		p_dx11->pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(p_texture_2d.GetAddressOf()));

		p_dx11->pDevice->CreateRenderTargetView(p_texture_2d.Get(), nullptr, p_dx11->pRenderTargetView.GetAddressOf());

		//If using DepthStencil reset it here and recreate the Depth stencil View at least, state can be ignored.

		//Currently no DepthStencil is not being considered
		p_dx11->pDeviceContext->OMSetRenderTargets(1, p_dx11->pRenderTargetView.GetAddressOf(), nullptr);

		D3D11_VIEWPORT d_viewport{};
		d_viewport.TopLeftX = 0.f;
		d_viewport.TopLeftY = 0.f;
		d_viewport.Width	= static_cast<float>(width);
		d_viewport.Height	= static_cast<float>(height);
		d_viewport.MinDepth = 0.f;
		d_viewport.MaxDepth = 1.f;

		p_dx11->pDeviceContext->RSSetViewports(1, &d_viewport);

		p_dx11->SetNewWindowSize(width, height);

		p_dx11->GetForegroundRenderList()->Clear();
		p_dx11->GetForegroundRenderList()->Clear();

		p_dx11->pConstantProjectionMatrixBuffer.Reset();
		p_dx11->pVertexBuffer.Reset();

		p_dx11->InitializeVertexBuffer();
		p_dx11->SetConstantBuffer();

		p_dx11->isBeingResized = false;
	}
}

void DX11::WindowFullscreen(bool isFullscreen) noexcept
{
	DX11* p_dx11 = Get();

	if (p_dx11->pSwapChain)
	{
		p_dx11->isBeingResized = true;

		p_dx11->pSwapChain->SetFullscreenState(isFullscreen, nullptr);

		(void)p_dx11->pRenderTargetView.Reset();

		if (FAILED(p_dx11->pSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0)))
		{
			OutputDebugStringW(L"\n\n\nError on ResizeBuffers\n\n\n");
			p_dx11->isBeingResized = false;
			return;
		}

		Microsoft::WRL::ComPtr<ID3D11Texture2D> p_texture_2d = nullptr;
		p_dx11->pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(p_texture_2d.GetAddressOf()));

		p_dx11->pDevice->CreateRenderTargetView(p_texture_2d.Get(), nullptr, p_dx11->pRenderTargetView.GetAddressOf());

		p_dx11->pDeviceContext->OMSetRenderTargets(1, p_dx11->pRenderTargetView.GetAddressOf(), nullptr);

		D3D11_TEXTURE2D_DESC texture_2d_desc {};
		p_texture_2d->GetDesc(&texture_2d_desc);

		D3D11_VIEWPORT d_viewport{};
		d_viewport.TopLeftX = 0.f;
		d_viewport.TopLeftY = 0.f;
		d_viewport.Width = static_cast<float>(texture_2d_desc.Width);
		d_viewport.Height = static_cast<float>(texture_2d_desc.Height);
		d_viewport.MinDepth = 0.f;
		d_viewport.MaxDepth = 1.f;

		p_dx11->pDeviceContext->RSSetViewports(1, &d_viewport);

		p_dx11->SetNewWindowSize(texture_2d_desc.Width, texture_2d_desc.Height);

		p_dx11->GetForegroundRenderList()->Clear();
		p_dx11->GetForegroundRenderList()->Clear();

		p_dx11->pConstantProjectionMatrixBuffer.Reset();
		p_dx11->pVertexBuffer.Reset();

		p_dx11->InitializeVertexBuffer();
		p_dx11->SetConstantBuffer();

		p_dx11->isBeingResized = false;
	}
}

bool DX11::Initialize(HWND window_handle, int window_width, int window_height)
{
	windowHandle = window_handle;
	windowWidth  = window_width;
	windowHeight = window_height;

	return Initialize();
}

bool DX11::Initialize()
{
	if(windowHandle == nullptr)
	{
		WindowManager const& window_manager = WindowManager::Get();

		windowHandle = window_manager.GetWindowHandle();
		windowHeight = window_manager.GetWindowHeight();
		windowWidth  = window_manager.GetWindowWidth();
	}

	if (!VerifyCpuSupport())
	{
		OutputDebugStringW(L"\n\n\nError Verifying Cpu Support\n\n\n");
		return false;
	}

	if (!VerifyAndSetRefreshRate(DXGI_FORMAT_R8G8B8A8_UNORM))
	{
		OutputDebugStringW(L"\n\n\nError Getting Refresh Rate\n\n\n");
		return false;
	}

	if(!InitializeDevices())
	{
		OutputDebugStringW(L"\n\n\nError Initializing Devices\n\n\n");
		return false;
	}

	if(!InitializeBackBuffer())
	{
		OutputDebugStringW(L"\n\n\nError Initializing BackBuffer\n\n\n");
		return false;
	}

	if(!InitializeRasterizerState())
	{
		OutputDebugStringW(L"\n\n\nError Initializing RasterizerState\n\n\n");
		return false;
	}

	if(!InitializePixelShader())
	{
		OutputDebugStringW(L"\n\n\nError Initializing PixelShader\n\n\n");
		return false;
	}

	if(!InitializeVertexShader())
	{
		OutputDebugStringW(L"\n\n\nError Initializing VertexShader\n\n\n");
		return false;
	}

	if(!InitializeVertexBuffer())
	{
		OutputDebugStringW(L"\n\n\nError Initializing VertexBuffer\n\n\n");
		return false;
	}

	//Creates RenderTarget with the DepthStencilView and BackBuffer
	SetBackBufferRenderTarget();
	SetViewPorts();
	SetShaders();
	SetConstantBuffer();
	SetInputLayout();
	SetBlendState();
	SetCommonState();

	InitializeFont();

	return false;
}

void DX11::Begin()
{
	if (isBeingResized)
		return;

	static constexpr float color[4] = { 0, 0, 0, 0 };
	pDeviceContext->ClearRenderTargetView(pRenderTargetView.Get(), color);

	//Required due to Text Drawing which is using own shaders
	pDeviceContext->PSSetShader(pPixelShader.Get(), nullptr, 0);
	pDeviceContext->VSSetShader(pVertexShader.Get(), nullptr, 0);

	pDeviceContext->IASetInputLayout(pInputLayout.Get());

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	pDeviceContext->IASetVertexBuffers(0, 1, pVertexBuffer.GetAddressOf(), &stride, &offset);

}

void DX11::End()
{
	pBackGroundRenderList->Draw();

	spriteBatch->Begin(DirectX::SpriteSortMode_Deferred, pBlendState.Get(), nullptr, nullptr, pRasterizerState.Get(), nullptr);
	pBackGroundRenderList->Draw2DText();
	spriteBatch->End();

	pDeviceContext->PSSetShader(pPixelShader.Get(), nullptr, 0);
	pDeviceContext->VSSetShader(pVertexShader.Get(), nullptr, 0);
	pDeviceContext->IASetInputLayout(pInputLayout.Get());

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	pDeviceContext->IASetVertexBuffers(0, 1, pVertexBuffer.GetAddressOf(), &stride, &offset);

	pForegroundRenderList->Draw();

	spriteBatch->Begin(DirectX::SpriteSortMode_Deferred, pNonPremultipliedState.Get(), nullptr, nullptr, pRasterizerState.Get(), nullptr);
	pTextureRenderList->DrawTexture();
	pForegroundRenderList->Draw2DText();
	spriteBatch->End();

	pDeviceContext->OMSetBlendState(pBlendState.Get(), nullptr, 0xffffffff);

	pSwapChain->Present(v_sync, 0);

	pBackGroundRenderList->Clear();
	pForegroundRenderList->Clear();
	pTextureRenderList->Clear();
}

RenderList* DX11::GetBackGroundRenderList() const noexcept
{
	return pBackGroundRenderList.get();
}

RenderList* DX11::GetForegroundRenderList() const noexcept
{
	return pForegroundRenderList.get();
}

TextureManager* DX11::GetTextureRenderList() const noexcept
{
	return pTextureRenderList.get();
}

AudioManager* DX11::GetAudioManager() const noexcept
{
	return pAudioManager.get();
}

void DX11::SetNewWindowSize(int window_width, int window_height)
{
	windowHeight = window_height;
	windowWidth  = window_width;
}

void DX11::SetVSync(unsigned int sync) noexcept
{
	v_sync = std::clamp(sync, 1u, 3u);
}

void DX11::DisableVSync() noexcept
{
	v_sync = 0;
}

void DX11::ChangeFont(std::uint8_t* const pData, std::size_t size) noexcept
{
	//just in case if being called while rendering
	isBeingResized = true;

	spriteBatch.reset();
	spriteFont.reset();

	spriteBatch = std::make_unique<DirectX::SpriteBatch>(pDeviceContext.Get());
	spriteFont = std::make_unique<DirectX::SpriteFont>(pDevice.Get(), pData, size, false);

	isBeingResized = false;
}

void DX11::AddVertices(RenderList* const pRenderList, std::span<Vertex> vertices, D3D11_PRIMITIVE_TOPOLOGY topology) const 
{
	if (vertices.size() > RenderList::MaxVertices)
		pRenderList->ClampBuffer();

	pRenderList->batches.emplace_back(vertices.size(), topology);

	pRenderList->vertices.resize(pRenderList->vertices.size() + vertices.size());
	std::memcpy(&pRenderList->vertices[pRenderList->vertices.size() - vertices.size()], vertices.data(), vertices.size() * sizeof(Vertex));
}

bool DX11::InitializeDevices()
{
	//For now VSync is standard on
	DXGI_MODE_DESC mode_desc {};
	mode_desc.Width				= windowWidth;
	mode_desc.Height			= windowHeight;
	mode_desc.RefreshRate		= RefreshRate;
	mode_desc.Format			= DXGI_FORMAT_R8G8B8A8_UNORM;
	mode_desc.ScanlineOrdering  = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	mode_desc.Scaling			= DXGI_MODE_SCALING_UNSPECIFIED;

	DXGI_SAMPLE_DESC sample_desc {};
	sample_desc.Count   = 4;
	sample_desc.Quality = 0;

	//For now only windowed mode only! Fullscreen support will come later
	DXGI_SWAP_CHAIN_DESC swap_chain_desc {};
	swap_chain_desc.BufferDesc		= mode_desc;
	swap_chain_desc.SampleDesc		= sample_desc;
	swap_chain_desc.BufferUsage		= DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_chain_desc.BufferCount		= 1;
	swap_chain_desc.OutputWindow	= windowHandle;
	swap_chain_desc.Windowed		= true;
	swap_chain_desc.SwapEffect		= DXGI_SWAP_EFFECT_DISCARD;
	swap_chain_desc.Flags			= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // Allowing to switch to Fullscreen when implemented -> IDXGISwapChain::ResizeTarget.

	D3D_FEATURE_LEVEL feature_level {};

	//Requires Direct3D 11.1 runtime
	constexpr D3D_FEATURE_LEVEL feature_levels[7]
	{
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
			D3D_FEATURE_LEVEL_9_3,
			D3D_FEATURE_LEVEL_9_2,
			D3D_FEATURE_LEVEL_9_1,
	};

	//Default Adapter for now
	HRESULT f_hresult = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
													  NULL, feature_levels, std::size(feature_levels),
													  D3D11_SDK_VERSION, &swap_chain_desc, pSwapChain.GetAddressOf(),
													  pDevice.GetAddressOf(), &feature_level, pDeviceContext.GetAddressOf());

	if(f_hresult == DXGI_ERROR_NOT_CURRENTLY_AVAILABLE)
	{
		OutputDebugStringW(L"\n\n\nCannot Initialize DXGI on a Session 0 process\n\n\n");
		return false;
	}

	if(f_hresult != S_OK)
	{
		OutputDebugStringW(L"\n\n\nCall to D3D11CreateDeviceAndSwapChain failed\n\n\n");
		return false;
	}


	return true;
}

bool DX11::InitializeBackBuffer()
{
	Microsoft::WRL::ComPtr<ID3D11Texture2D> pID3D11Texture2D = nullptr;
	HRESULT buffer_result = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(pID3D11Texture2D.GetAddressOf()));

	if(buffer_result != S_OK)
	{
		OutputDebugStringW(L"\n\n\nCannot Get Back Buffer from the Swap Chain\n\n\n");
		return false;
	}

	HRESULT render_view_result = pDevice->CreateRenderTargetView(pID3D11Texture2D.Get(), nullptr, pRenderTargetView.GetAddressOf());

	if (render_view_result != S_OK)
	{
		OutputDebugStringW(L"\n\n\nCannot CreateRenderTargetView\n\n\n");
		return false;
	}

	return true;
}

bool DX11::InitializeRasterizerState()
{
	D3D11_RASTERIZER_DESC rasterizer_desc {};
	rasterizer_desc.AntialiasedLineEnable	= false;
	rasterizer_desc.CullMode				= D3D11_CULL_BACK;
	rasterizer_desc.DepthBias				= 0;
	rasterizer_desc.DepthBiasClamp			= 0.0f;
	rasterizer_desc.DepthClipEnable			= true;
	rasterizer_desc.FillMode				= D3D11_FILL_SOLID;
	rasterizer_desc.FrontCounterClockwise	= false;
	rasterizer_desc.MultisampleEnable		= false;
	rasterizer_desc.ScissorEnable			= false;
	rasterizer_desc.SlopeScaledDepthBias	= 0.0f;

	HRESULT rasterizer_result = pDevice->CreateRasterizerState(&rasterizer_desc, pRasterizerState.GetAddressOf());

	if(rasterizer_result != S_OK)
	{
		OutputDebugStringW(L"\n\n\nCannot CreateRasterizerState\n\n\n");
		return false;
	}

	pDeviceContext->RSSetState(pRasterizerState.Get());

	return true;
}

bool DX11::InitializePixelShader()
{
	static constexpr char pixel_shader_code[]
	{
		R"(
			struct PixelInputType
			{
			    float4 position : SV_POSITION;
			    float4 color : COLOR;
			};

			float4 ColorPixelShader(PixelInputType input) : SV_Target
			{
			    return input.color;
			}
		)"
	};

	HRESULT compile_result = D3DCompile(pixel_shader_code, std::size(pixel_shader_code), "PixelShader", nullptr,
										nullptr, "ColorPixelShader", "ps_5_0", NULL,
										NULL, pPixelShaderBlob.GetAddressOf(), nullptr);

	if(compile_result != S_OK)
	{
		OutputDebugStringW(L"\n\n\nCannot Compile Pixel Shader\n\n\n");
		return false;
	}

	HRESULT result_pixel_shader = pDevice->CreatePixelShader(pPixelShaderBlob->GetBufferPointer(), pPixelShaderBlob->GetBufferSize(), nullptr, pPixelShader.GetAddressOf());

	if (result_pixel_shader != S_OK)
	{
		OutputDebugStringW(L"\n\n\nCannot CreatePixelShader\n\n\n");
		return false;
	}

	return true;
}

bool DX11::InitializeVertexShader()
{
	static constexpr char vertex_shader_code[]
	{
		R"(
			struct VS_INPUT
			{
			    float3 position : POSITION; 
			    float4 color : COLOR;       
			};

			struct VS_OUTPUT
			{
			    float4 position : SV_POSITION; 
			    float4 color : COLOR;          
			};

			cbuffer TransformBuffer : register(b0)
			{
			    matrix worldViewProj;  
			};

			VS_OUTPUT main(VS_INPUT input)
			{
			    VS_OUTPUT output;

			    output.position = mul(worldViewProj, float4(input.position, 1.0f));
			    output.color = float4(input.color.rgb / 255.0f, input.color.a); //divide 255 for normalized colors to support simple RGB google paste, keep a for blend state
			
			    return output;
			}
		)"
	};

	HRESULT compile_result = D3DCompile(vertex_shader_code, std::size(vertex_shader_code), "VertexShader", nullptr,
										nullptr, "main", "vs_5_0", NULL,
										NULL, pVertexShaderBlob.GetAddressOf(), nullptr);

	if (compile_result != S_OK)
	{
		OutputDebugStringW(L"\n\n\nCannot Compile Pixel Shader\n\n\n");
		return false;
	}

	HRESULT result_vertex_shader = pDevice->CreateVertexShader(pVertexShaderBlob->GetBufferPointer(), pVertexShaderBlob->GetBufferSize(), nullptr, pVertexShader.GetAddressOf());

	if (result_vertex_shader != S_OK)
	{
		OutputDebugStringW(L"\n\n\nCannot CreateVertexShader\n\n\n");
		return false;
	}

	return true;
}

bool DX11::InitializeVertexBuffer()
{
	D3D11_BUFFER_DESC buffer_desc{};
	buffer_desc.ByteWidth			= sizeof(Vertex) * static_cast<UINT>(RenderList::MaxVertices);
	buffer_desc.Usage				= D3D11_USAGE_DYNAMIC;
	buffer_desc.BindFlags			= D3D11_BIND_VERTEX_BUFFER;
	buffer_desc.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;
	buffer_desc.MiscFlags			= 0;
	buffer_desc.StructureByteStride = 0;

	HRESULT constant_buffer_status = pDevice->CreateBuffer(&buffer_desc, nullptr, pVertexBuffer.GetAddressOf());

	if (constant_buffer_status != S_OK)
	{
		OutputDebugStringW(L"\n\n\nCannot CreateBuffer\n\n\n");
		return false;
	}

	return true;
}

void DX11::InitializeFont()
{
	spriteBatch = std::make_unique<DirectX::SpriteBatch>(pDeviceContext.Get());
	spriteFont = std::make_unique<DirectX::SpriteFont>(pDevice.Get(), DefaultFontBuffer, sizeof(DefaultFontBuffer), false);
}

void DX11::SetCommonState()
{
	common_states = std::make_unique<DirectX::CommonStates>(pDevice.Get());
	pNonPremultipliedState = common_states->NonPremultiplied();
}

void DX11::SetConstantBuffer()
{
	D3D11_BUFFER_DESC buffer_desc {};
	buffer_desc.ByteWidth			= sizeof(DirectX::SimpleMath::Matrix);
	buffer_desc.Usage				= D3D11_USAGE_DYNAMIC;
	buffer_desc.BindFlags			= D3D11_BIND_CONSTANT_BUFFER;
	buffer_desc.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;
	buffer_desc.MiscFlags			= 0;
	buffer_desc.StructureByteStride = 0;

	UINT numViewports = 1;
	D3D11_VIEWPORT viewport{};
	pDeviceContext->RSGetViewports(&numViewports, &viewport);

	projection_matrix = DirectX::SimpleMath::Matrix::CreateOrthographicOffCenter(viewport.TopLeftX, viewport.Width, viewport.Height, viewport.TopLeftY,
		viewport.MinDepth, viewport.MaxDepth);

	D3D11_SUBRESOURCE_DATA subresource_data {};
	subresource_data.pSysMem = &projection_matrix;

	HRESULT constant_buffer_status = pDevice->CreateBuffer(&buffer_desc, &subresource_data, pConstantProjectionMatrixBuffer.GetAddressOf());

	if (constant_buffer_status != S_OK)
	{
		OutputDebugStringW(L"\n\n\nCannot CreateBuffer\n\n\n");
		return;
	}

	pDeviceContext->VSSetConstantBuffers(0, 1, pConstantProjectionMatrixBuffer.GetAddressOf());
}

void DX11::SetInputLayout()
{
	constexpr D3D11_INPUT_ELEMENT_DESC input_element_desc[2]
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	HRESULT input_layout_result = pDevice->CreateInputLayout(&input_element_desc[0], std::size(input_element_desc), pVertexShaderBlob->GetBufferPointer(),
		pVertexShaderBlob->GetBufferSize(), pInputLayout.GetAddressOf());

	if(input_layout_result != S_OK)
	{
		OutputDebugStringW(L"\n\n\nCannot CreateInputLayout\n\n\n");
		return;
	}

	pDeviceContext->IASetInputLayout(pInputLayout.Get());
}

void DX11::SetBlendState()
{
	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.AlphaToCoverageEnable = 0;
	blendDesc.IndependentBlendEnable = 0;

	blendDesc.RenderTarget[0].BlendEnable = 1;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	HRESULT blend_result = pDevice->CreateBlendState(&blendDesc, pBlendState.GetAddressOf());

	if(blend_result != S_OK)
	{
		OutputDebugStringW(L"\n\n\nCannot CreateBlendState\n\n\n");
		return;
	}

	pDeviceContext->OMSetBlendState(pBlendState.Get(), nullptr, 0xffffffff);
}

bool DX11::VerifyCpuSupport() const noexcept
{
	if (!DirectX::XMVerifyCPUSupport())
	{
		MessageBoxW(nullptr, L"Cpu is not supported", L"Fatal Error", NULL);
		return false;
	}
	return true;
}

bool DX11::VerifyAndSetRefreshRate(DXGI_FORMAT format)
{
	//Primary Monitor
	unsigned int screenWidth  = GetSystemMetrics(SM_CXSCREEN);
	unsigned int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	Microsoft::WRL::ComPtr<IDXGIFactory> pIDXGIFactory = nullptr;

	if (CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(pIDXGIFactory.GetAddressOf())) != S_OK)
	{
		OutputDebugStringW(L"\n\n\nError Creating DXGIFactory\n\n\n");
		return false;
	}

	IDXGIAdapter* p_temp_adapter = nullptr;
	std::vector<Microsoft::WRL::ComPtr<IDXGIAdapter>> adapters;

	for (size_t index = 0ull; pIDXGIFactory->EnumAdapters(static_cast<UINT>(index), &p_temp_adapter) != DXGI_ERROR_NOT_FOUND; index++)
	{
		if(p_temp_adapter != nullptr)
		{
			adapters.emplace_back(p_temp_adapter);
			p_temp_adapter = nullptr;
		}
	}

	IDXGIOutput* p_temp_adapter_output = nullptr;
	std::vector<Microsoft::WRL::ComPtr<IDXGIOutput>> adapter_outputs;

	for (size_t index = 0ull; adapters.at(index)->EnumOutputs(static_cast<UINT>(index), &p_temp_adapter_output) != DXGI_ERROR_NOT_FOUND; index++)
	{
		if(p_temp_adapter_output != nullptr)
		{
			adapter_outputs.emplace_back(p_temp_adapter_output);
			p_temp_adapter_output = nullptr;
		}
	}

	auto GetNumDisplayModes = [format](Microsoft::WRL::ComPtr<IDXGIOutput> const& gi_Output) -> UINT
	{
		UINT num = 0;
		gi_Output->GetDisplayModeList(format, DXGI_ENUM_MODES_INTERLACED, &num, nullptr);
		return num;
	};

	bool adapter_found = false;
	size_t adapter_index = 0;

	for(Microsoft::WRL::ComPtr<IDXGIOutput> const& gi_Output : adapter_outputs)
	{
		UINT display_modes_count = GetNumDisplayModes(gi_Output);
		std::unique_ptr<DXGI_MODE_DESC[]> buffer = std::make_unique_for_overwrite<DXGI_MODE_DESC[]>(display_modes_count);

		gi_Output->GetDisplayModeList(format, DXGI_ENUM_MODES_INTERLACED, &display_modes_count, buffer.get());

		DXGI_MODE_DESC* pModeDescArray = buffer.get();

		for (size_t index = 0ull; index < display_modes_count; index++)
		{
			DXGI_MODE_DESC pModeDesc = pModeDescArray[index];

			if(pModeDesc.Width == screenWidth && pModeDesc.Height == screenHeight)
			{
				RefreshRate = pModeDesc.RefreshRate;
				adapter_found = true;
				break;
			}
		}

		if (adapter_found)
			break;

		++adapter_index;
	}

	if(adapter_found)
	{
		std::unique_ptr<DXGI_ADAPTER_DESC> buffer = std::make_unique_for_overwrite<DXGI_ADAPTER_DESC>();
		adapters.at(adapter_index)->GetDesc(buffer.get());

		//In MegaBytes
		AdapterVirtualMemory = buffer.get()->DedicatedVideoMemory / (1 << 10 * 1 << 10);

		if (AdapterVirtualMemory)
			return true;
	}

	return false;
}

void DX11::SetBackBufferRenderTarget() const noexcept
{
	//For now NumViews will always be 1
	pDeviceContext->OMSetRenderTargets(1, pRenderTargetView.GetAddressOf(), nullptr);
}

void DX11::SetViewPorts() const noexcept
{
	D3D11_VIEWPORT d_viewport {};
	d_viewport.TopLeftX = 0.f;
	d_viewport.TopLeftY = 0.f;
	d_viewport.Width	= static_cast<float>(windowWidth);
	d_viewport.Height	= static_cast<float>(windowHeight);
	d_viewport.MinDepth = 0.f;
	d_viewport.MaxDepth = 1.f;

	pDeviceContext->RSSetViewports(1, &d_viewport);
}

void DX11::SetShaders() const noexcept
{
	pDeviceContext->PSSetShader(pPixelShader.Get(), nullptr, 0);
	pDeviceContext->VSSetShader(pVertexShader.Get(), nullptr, 0);
}

void RenderList::Draw() const noexcept
{
	if (vertices.empty())
		return;

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	pRenderer->pDeviceContext->Map(pRenderer->pVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	std::memcpy(mappedResource.pData, vertices.data(), vertices.size() * sizeof(Vertex));
	pRenderer->pDeviceContext->Unmap(pRenderer->pVertexBuffer.Get(), 0);

	std::size_t pos = 0;
	for (auto const& batch : batches)
	{
		pRenderer->pDeviceContext->IASetPrimitiveTopology(batch.topology);
		pRenderer->pDeviceContext->Draw(static_cast<UINT>(batch.count), static_cast<UINT>(pos));

		pos += batch.count;
	}
}

void RenderList::DrawLine(Vector2 from, Vector2 to, Color const& color) noexcept
{
	Vertex _vertices[2]
	{
		{ {from.x, from.y, 0.f} , color },
		{ {to.x, to.y, 0.f}, color }
	};

	pRenderer->AddVertices(this, _vertices, D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
}

void RenderList::DrawDot(Vector2 pos, Color const& color) noexcept
{
	DrawFilledRect({ pos.x, pos.y, 2.f, 2.f }, color);
}

void RenderList::DrawFilledRect(Vector4 const& rect, Color const& color) noexcept
{
	Vertex _vertices[6]
	{
		{ {rect.x, rect.y, 0.f}, color },
		{ {rect.x + rect.z, rect.y, 0.f}, color },
		{ {rect.x, rect.y + rect.w, 0.f}, color },

		{ {rect.x + rect.z, rect.y, 0.f}, color },
		{ {rect.x + rect.z, rect.y + rect.w, 0.f}, color },
		{ {rect.x, rect.y + rect.w, 0.f}, color }
	};

	pRenderer->AddVertices(this, _vertices, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void RenderList::DrawRect(Vector4 const& rect, float strokeWidth, Color const& color) noexcept
{
	DirectX::XMFLOAT4 tmp = rect;
	tmp.z = strokeWidth;
	DrawFilledRect(tmp, color);
	tmp.x = rect.x + rect.z - strokeWidth;
	DrawFilledRect(tmp, color);
	tmp.z = rect.z;
	tmp.x = rect.x;
	tmp.w = strokeWidth;
	DrawFilledRect(tmp, color);
	tmp.y = rect.y + rect.w;
	DrawFilledRect(tmp, color);
}

void RenderList::DrawOutlinedFilledRect(Vector4 const& rect, float strokeWidth, Color const& OutlineColor, Color const& color) noexcept
{
	DrawFilledRect(rect, color);
	DrawRect(rect, strokeWidth, OutlineColor);
}

void RenderList::DrawFilledRectGradient(Vector4 const& rect, Color color, float gradientIntensity) noexcept
{
	float scaleFactor = 1.0f - gradientIntensity;

	Color color2 = Color{ color.x * (0.8f + scaleFactor * 0.2f), color.y * (0.8f + scaleFactor * 0.2f), color.z * (0.8f + scaleFactor * 0.2f), color.w };
	Color color3 = Color{ color.x * (0.6f + scaleFactor * 0.4f), color.y * (0.6f + scaleFactor * 0.4f), color.z * (0.6f + scaleFactor * 0.4f), color.w };
	Color color4 = Color{ color.x * (0.4f + scaleFactor * 0.6f), color.y * (0.4f + scaleFactor * 0.6f), color.z * (0.4f + scaleFactor * 0.6f), color.w };

	Vertex _vertices[6]
	{
		{ {rect.x, rect.y, 0.f}, color },     
		{ {rect.x + rect.z, rect.y, 0.f}, color2 },
		{ {rect.x, rect.y + rect.w, 0.f}, color3 }, 

		{ {rect.x + rect.z, rect.y, 0.f}, color2 },
		{ {rect.x + rect.z, rect.y + rect.w, 0.f}, color4 }, 
		{ {rect.x, rect.y + rect.w, 0.f}, color3 }, 
	};

	pRenderer->AddVertices(this, _vertices, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void RenderList::DrawOutlinedRect(Vector4 const& rect, float strokeWidth, Color const& OutlineColor) noexcept
{
	DirectX::XMFLOAT4 tmp = rect;
	tmp.z = strokeWidth;  
	DrawFilledRect(tmp, OutlineColor);
	tmp.x = rect.x + rect.z - strokeWidth;  
	DrawFilledRect(tmp, OutlineColor);
	tmp = rect;
	tmp.w = strokeWidth;  
	DrawFilledRect(tmp, OutlineColor);
	tmp.y = rect.y + rect.w - strokeWidth; 
	DrawFilledRect(tmp, OutlineColor);
}

void RenderList::DrawCircle(Vector2 pos, float radius, Color const& color) noexcept
{
	constexpr int segments = 24;

	Vertex _vertices[segments + 1];

	constexpr float angleIncrement = DirectX::XM_2PI / segments;

	float x = radius, y = 0.0f;

	float cosTheta = std::cos(angleIncrement);
	float sinTheta = std::sin(angleIncrement);

	for (int i = 0; i <= segments; ++i) {

		_vertices[i] = Vertex{
			{pos.x + x, pos.y + y, 0.f},
			color
		};

		float newX = x * cosTheta - y * sinTheta;
		y = x * sinTheta + y * cosTheta;
		x = newX;
	}

	pRenderer->AddVertices(this, _vertices, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
}

void RenderList::Draw2DText() const noexcept
{
	if (text_buffer.empty() || text_data.empty())
		return;

	if (text_buffer.size() != text_data.size())
		return;

	DirectX::SpriteBatch* p_SpriteBatch = pRenderer->spriteBatch.get();

	std::size_t pos = 0;
	for (std::pair<std::string, Vector2> const& text : text_buffer)
	{
		pRenderer->spriteFont->DrawString(p_SpriteBatch, text.first.c_str(), text.second, text_data[pos].first, 0.f, {}, text_data[pos].second);
		++pos;
	}

}

void RenderList::DrawString(const char* string, Vector2 pos, Color color, float scale) noexcept
{
	static constexpr float normal_color_conversion = 1.f / 255.f;
	color = Color{color.x * normal_color_conversion, color.y * normal_color_conversion, color.z * normal_color_conversion, color.w};

	text_buffer.emplace_back(string, pos);
	text_data.emplace_back(color, scale);
}

RenderList::Vector2 RenderList::MeasureString(const char* string, bool ignore_whitespace) const noexcept
{
	return pRenderer->spriteFont->MeasureString(string, ignore_whitespace);
}

void TextureManager::ClampTextureBuffer() noexcept
{
	if (textures.size() > MaxTextureCount)
		textures.erase(std::next(textures.begin(), MaxTextureCount), textures.end());
}

void TextureManager::ClearAllTextures() noexcept
{
	textures.clear();
}

void TextureManager::OnResize() noexcept
{
	std::vector<std::pair<const int, std::size_t>> temp_texture_data;

	for (std::pair<const int, TextureData>& curr : textures)
		temp_texture_data.emplace_back(curr.first, curr.second.buffer_size);

	ClearAllTextures();

	for (std::pair<const int, void const*> const& memory_texture_file : memory_texture_file_map)
	{
		for (std::pair<const int, std::size_t>& curr : temp_texture_data)
		{
			if(curr.first == memory_texture_file.first)
			{
				TextureData texture_data;
				HRESULT result = DirectX::CreateWICTextureFromMemory(pRenderer->pDevice.Get(), static_cast<std::uint8_t const*>(memory_texture_file.second), curr.second,
					texture_data.pTextureResource.GetAddressOf(), texture_data.pTextureResourceView.GetAddressOf());

				if (result != S_OK)
					OutputDebugStringW(L"\n\n\nError on CreateWICTextureFromFile\n\n\n");

				texture_data.buffer_size = curr.second;
				textures[curr.first] = texture_data;

				break;
			}
		}
	}

	for (std::pair<const int, std::wstring>& texture_file : texture_file_map)
	{
		for (std::pair<const int, std::size_t>& curr : temp_texture_data)
		{
			if(curr.first == texture_file.first)
			{
				TextureData texture_data;
				HRESULT result = DirectX::CreateWICTextureFromFile(pRenderer->pDevice.Get(), texture_file.second.c_str(),
					texture_data.pTextureResource.GetAddressOf(), texture_data.pTextureResourceView.GetAddressOf());

				if (result != S_OK)
					OutputDebugStringW(L"\n\n\nError on CreateWICTextureFromFile\n\n\n");

				textures[curr.first] = texture_data;

				break;
			}
		}
	}
}

int TextureManager::AddTexture(const wchar_t* file) noexcept
{
	for (const auto& pair : texture_file_map)
	{
		if (pair.second == file)
		{
			return pair.first;  
		}
	}

	TextureData texture_data;
	HRESULT result = DirectX::CreateWICTextureFromFile(pRenderer->pDevice.Get(), file, texture_data.pTextureResource.GetAddressOf(), texture_data.pTextureResourceView.GetAddressOf());

	if (result != S_OK)
	{
		OutputDebugStringW(L"\n\n\nError on CreateWICTextureFromFile\n\n\n");
		return -1;
	}

	int texture_id = internal_texture_id++;
	textures[texture_id] = texture_data;
	texture_file_map[texture_id] = file;

	return texture_id;
}

int TextureManager::AddTexture(const char* file) noexcept
{
	std::string temp { file };
	std::wstring wconverted { temp.begin(), temp.end() };
	return AddTexture(wconverted.c_str());
}

int TextureManager::AddTexture(const uint8_t* buffer, std::size_t size) noexcept
{
	for (const auto& pair : memory_texture_file_map)
	{
		if (pair.second == buffer)
		{
			return pair.first;
		}
	}

	TextureData texture_data;
	HRESULT result = DirectX::CreateWICTextureFromMemory(pRenderer->pDevice.Get(), buffer, size, texture_data.pTextureResource.GetAddressOf(), texture_data.pTextureResourceView.GetAddressOf());

	if (result != S_OK)
	{
		OutputDebugStringW(L"\n\n\nError on CreateWICTextureFromFile\n\n\n");
		return -1;
	}

	texture_data.buffer_size = size;

	int texture_id = internal_texture_id++;
	textures[texture_id] = texture_data;
	memory_texture_file_map[texture_id] = reinterpret_cast<void const*>(buffer);

	return texture_id;
}

void TextureManager::DrawTexture(Vector2 pos, Color color, int texture_id, Vector2 scale)
{
	if (textures.find(texture_id) != textures.end())
		texture_draw_data.emplace_back(pos, scale, color, texture_id);
}

void TextureManager::DrawTexture() noexcept
{
	if (texture_draw_data.empty())
		return;

	DirectX::SpriteBatch* pSpriteBatch = pRenderer->spriteBatch.get();
	for (TextureDrawData const& texture : texture_draw_data)
		pSpriteBatch->Draw(textures.at(texture.texture_id).pTextureResourceView.Get(), texture.pos, nullptr, texture.color, 0.f, {}, texture.texture_scaling);
}

void TextureManager::Clear() noexcept
{
	if (!texture_draw_data.empty())
		texture_draw_data.clear();
}

bool AudioManager::InitializeAudioManager()
{
	HRESULT result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	if (result != S_OK)
	{
		OutputDebugStringW(L"\n\n\nError CoInitializeEx Audio Manager\n\n\n");
		return false;
	}

	pAudioEngine = std::make_unique<DirectX::AudioEngine>();

	return true;
}

int AudioManager::AddSoundEffect(const wchar_t* wavFile)
{
	if(wavFile == nullptr)
	{
		OutputDebugStringW(L"\n\n\nSpecified File was nullptr\n\n\n");
		return -1;
	}

	if (internal_audio_id + 1 > MaxAudioCount)
	{
		OutputDebugStringW(L"\n\n\nError Maximum Audio Limit Reached!\n\n\n");
		return -1;
	}

	SoundData sound_data {};
	sound_data.wav_file = wavFile;
	sound_data.pSoundEffect = std::make_unique<DirectX::SoundEffect>(pAudioEngine.get(), wavFile);

	int sound_id = internal_audio_id++;
	sound_effects[sound_id] = std::move(sound_data);

	return sound_id;
}

void AudioManager::PlaySoundEffect(int soundID, float volume, float pitch, float pan)
{
	if(soundID == -1)
	{
		OutputDebugStringW(L"\n\n\nFile was not added due to Maximum Audio Limit Reached!\n\n\n");
		return;
	}

	if(soundID < sound_effects.size())
		sound_effects[soundID].pSoundEffect->Play(volume, pitch, pan);
}
