%define name grip
%define version 2.98.0
%define release 1
%define prefix /usr
%define 

Summary: Grip, a CD player, ripper, and MP3-encoder front-end
Name: %{name}
Version: %{version}
Release: %{release}
Copyright: GPL
Group: Applications/Sound
Source: http://www.nostatic/grip/grip-%{version}.tar.gz
URL: http://www.nostatic.org/grip
Packager: Mike Oliphant <oliphant@gtk.org>
#Icon: gripicon.png
Buildroot: /tmp/%{name}-root

%description
Grip is a cd-player and cd-ripper for Gnome. It has the ripping capabilities
of cdparanoia built in, but can also use external rippers (such as
cdda2wav). It also provides an automated frontend for MP3 encoders, letting
you take a disc and transform it easily straight into MP3s. The CDDB
protocol is supported for retrieving track information from disc database
servers. Grip works with DigitalDJ to provide a unified "computerized"
version of your music collection.

%prep
rm -rf $RPM_BUILD_ROOT
%setup

%build
./configure --prefix=%{prefix} --disable-shared-cdpar --disable-shared-id3
make

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT

make prefix=$RPM_BUILD_ROOT%{prefix} install

%clean
rm -rf $RPM_BUILD_ROOT

%files
%doc README ChangeLog CREDITS COPYING INSTALL NEWS AUTHORS TODO ABOUT-NLS
%{prefix}/bin/grip
%{prefix}/share/gnome/help/grip
%{prefix}/share/gnome/apps/Multimedia/grip.desktop
%{prefix}/share/pixmaps/gripicon.png
