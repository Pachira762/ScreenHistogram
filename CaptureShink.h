#pragma once

class CaptureShink
{
public:
	virtual ~CaptureShink() {}

	virtual void	Shink(winrt::com_ptr<ID3D11Texture2D>& capture) = 0;
};
