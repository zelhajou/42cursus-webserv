#ifndef __KQUEUE_HPP__
# define __KQUEUE_HPP__

# include <cstring>
# include <sys/event.h>
# include <sys/time.h>
# include <unistd.h>
# include <stdexcept>

class KQueue {
public:
	KQueue();
	~KQueue();
	void	SET_QUEUE(int fd, short filter, bool mode);
	int	CHECK_QUEUE(struct kevent *events);
	int	get_current_events();

//private:
	int		kq;
	struct	kevent	event;
	int		current_events;
};

#endif
