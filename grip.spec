%define name grip
%define version 2.98.6
%define release 1
%define prefix /usr

Summary: Grip, a CD player, ripper, and MP3-encoder front-end
Summary(fr): Grip, un lecteur de CD, extracteur, et IHM pour encodeur MP3
Name: %{name}
Version: %{version}
Release: %{release}
Copyright: GPL
Group: Applications/Sound
Source: http://www.nostatic/grip/grip-%{version}.tar.gz
URL: http://www.nostatic.org/grip
Packager: Mike Oliphant <oliphant@gtk.org>
Icon: gripicon.gif
Buildroot: /tmp/%{name}-root
Epoch: 1

%description
Grip is a cd-player and cd-ripper for the Gnome desktop. It has the ripping
capabilities of cdparanoia built in, but can also use external rippers (such
as cdda2wav). It also provides an automated frontend for MP3 encoders,
letting you take a disc and transform it easily straight into MP3s. Internet
disc lookups are supported for retrieving track information from disc
database servers. Grip works with DigitalDJ to provide a unified
"computerized" version of your music collection.

%description -l fr
Grip est un lecteur-extracteur de CD pour le Bureau GNome. Il a des
capacités d'extraction du type cdparanoia incluses, mais il peut utiliser
un extracteur externe (comme cdda2wav). Il fournis également une IHM
pour les encodeurs MP3, permettant simplement de prendre un disque
et de le transformer en MP3. La récupération des noms sur Internet
permet également d'avoir les informations sur les pistes auprès des 
serveurs CDDB.
Grip fonctionne également avec DigitalDJ pour fournir une version unifiée
informatique de votre discothèque.


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

%changelog
* Mon Jan 28 2002 Eric Lassauge <lassauge@mail.dotcom.fr>
- Added french translations
