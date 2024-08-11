#include "KQueue.hpp"

KQueue::~KQueue() {
	close(this->kq);
}

KQueue::KQueue()
	: current_events(0) {
		this->kq = kqueue();
		if (this->kq < 0)	throw	1;
	}

int	KQueue::get_current_events() { return this->current_events; }

void	KQueue::SET_QUEUE(int fd, short filter, bool mode) {
	std::memset(&this->event, 0, sizeof(this->event));
	if (mode) {
		EV_SET(&this->event, fd, filter, EV_ADD|EV_ENABLE, 0, 0, 0);
		this->current_events += 1;
	}
	else {	
		EV_SET(&this->event, fd, filter, EV_DELETE, 0, 0, 0);
		this->current_events -= 1;
	}
	kevent(this->kq, &this->event, 1, NULL, 0, NULL);
}

int	KQueue::CHECK_QUEUE(struct kevent *events) {
	if (this->current_events <= 0)	return 0;
	return	kevent(this->kq, NULL, 0, events, this->current_events, NULL);
}
