#ifndef NET_EPOLL_SELECTOR_H
#define NET_EPOLL_SELECTOR_H

#include <stdlib.h>
#include <sys/epoll.h>
#include "net/fdmap.h"
#include "io/observer.h"
#include "io/event.h"

namespace net {
  class selector {
    public:
      // Constructor.
      selector();

      // Destructor.
      ~selector();

      // Set I/O observer.
      void set_io_observer(io::observer* obs);

      // Get size.
      size_t size() const;

      // Get count.
      size_t count() const;

      // Create.
      bool create();

      // Add descriptor.
      bool add(unsigned fd,
               fdtype type,
               io::event_handler* handler,
               io::event events);

      // Remove descriptor.
      bool remove(unsigned fd);

      // Wait for events.
      bool wait_for_events();
      bool wait_for_events(unsigned timeout); // Timeout in ms.

      // Process file descriptor's events.
      bool process_fd_events(unsigned fd,
                             fdtype type,
                             io::event_handler* handler,
                             io::event events);

      // Process all events.
      void process_events();

    private:
      fdmap _M_fdmap;

      int _M_fd;

      struct epoll_event* _M_events;

      unsigned _M_nevents;

      io::observer* _M_io_observer;

      // Disable copy constructor and assignment operator.
      selector(const selector&) = delete;
      selector& operator=(const selector&) = delete;
  };

  inline selector::selector()
    : _M_fd(-1),
      _M_events(NULL),
      _M_io_observer(NULL)
  {
  }

  inline void selector::set_io_observer(io::observer* obs)
  {
    _M_io_observer = obs;
  }

  inline size_t selector::size() const
  {
    return _M_fdmap.size();
  }

  inline size_t selector::count() const
  {
    return _M_fdmap.count();
  }

  inline bool selector::wait_for_events()
  {
    int ret;
    if ((ret = epoll_wait(_M_fd, _M_events, _M_fdmap.count(), -1)) <= 0) {
      return false;
    }

    _M_nevents = ret;

    return true;
  }

  inline bool selector::wait_for_events(unsigned timeout)
  {
    int ret;
    if ((ret = epoll_wait(_M_fd, _M_events, _M_fdmap.count(), timeout)) <= 0) {
      return false;
    }

    _M_nevents = ret;

    return true;
  }

  inline bool selector::process_fd_events(unsigned fd,
                                          fdtype type,
                                          io::event_handler* handler,
                                          io::event events)
  {
    if (handler->on_io(events) == io::event_handler::result::kError) {
      if (_M_io_observer) {
        _M_io_observer->on_error(handler);
      }

      return false;
    }

    if (_M_io_observer) {
      _M_io_observer->on_success(handler);
    }

    return true;
  }
}

#endif // NET_EPOLL_SELECTOR_H
