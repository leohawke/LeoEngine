#include "HUDPseudoWindow.h"

LEO_BEGIN

struct HUDPseudoWindow::HUDPseudoWindowImpl {

	HUDPseudoWindowImpl(){}

	~HUDPseudoWindowImpl(){}

	void Resize(const size_type& size) {

	}

	void Update(float t) {

	}

	void Render() {

	}
};

HUDPseudoWindow::HUDPseudoWindow(HUD::HostRenderer &)
	:pImpl(new HUDPseudoWindowImpl())
{
}

void HUDPseudoWindow::Resize(const size_type & size)
{
	pImpl->Resize(size);
}

void HUDPseudoWindow::Update(float t)
{
	pImpl->Update(t);
}

void HUDPseudoWindow::Render()
{
	pImpl->Render();
}

bool HUDPseudoWindow::IsMinimized()
{
	return false;
}

//because impl
HUDPseudoWindow::~HUDPseudoWindow()
{
}

LEO_END
