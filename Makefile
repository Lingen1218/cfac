#####################################################
# Top-level Makefile for cFAC                       #
#####################################################
# You should not change anything here.              #
#####################################################

TOP = .

include Make.conf

subdirs : configure Make.conf
	@set -e; for i in $(SUBDIRS); do (cd $$i; $(MAKE)) || exit 1; done

all : subdirs


install : subdirs
	@set -e; for i in $(SUBDIRS); do (cd $$i; $(MAKE) install) || exit 1; done

check : subdirs
	@set -e; for i in $(SUBDIRS); do (cd $$i; $(MAKE) check) || exit 1; done

clean :
	@set -e; for i in $(SUBDIRS); do (cd $$i; $(MAKE) clean) || exit 1; done
	$(RM) -r build

distclean : clean
	$(RM) config.log config.status config.cache include/config.h Make.conf
	$(RM) -r autom4te.cache

devclean : distclean
	$(RM) configure NEWS ChangeLog

texts : ChangeLog

ChangeLog : dummy
	git log --pretty=format:"- %s%n%b" > ChangeLog

Make.conf : ac-tools/Make.conf.in configure
	@echo
	@echo 'Please re-run ./configure'
	@echo
	@exit 1

configure : ac-tools/configure.ac
	autoconf -o $@ $<
	chmod +x $@

dummy :
