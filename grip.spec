%define name grip
%define version 2.99.3
%define release 1
%define prefix /usr

Summary: Grip, a CD player, ripper, and MP3-encoder front-end
Summary(zh_CN): Grip是一个 CD 播放器, 抓轨器和 MP3 编码器前端程序. 
Summary(fr): Grip, un lecteur de CD, extracteur, et IHM pour encodeur MP3
Name: %{name}
Version: %{version}
Release: %{release}
Copyright: GPL
Group: Applications/Multimedia
Source: http://www.nostatic/grip/grip-%{version}.tar.gz
URL: http://www.nostatic.org/grip
Packager: Mike Oliphant <oliphant@gtk.org>
Icon: gripicon.gif
Buildroot: /var/tmp/%{name}-%{version}-root
Epoch: 1

%description
Grip is a cd-player and cd-ripper for the Gnome desktop. It has the ripping
capabilities of cdparanoia built in, but can also use external rippers (such
as cdda2wav). It also provides an automated frontend for MP3 encoders,
letting you take a disc and transform it easily straight into MP3s. Internet
disc lookups are supported for retrieving track information from disc
database servers. Grip works with DigitalDJ to provide a unified
"computerized" version of your music collection.

%description -l zh_CN
Grip 是一个可以在Gnome桌面环境下运行的CD音乐播放器和抓轨器, 它可以使用
内置的 cdparanoia 程序抓轨器(将音轨存储为文件),也可以使用外部的抓轨器
(例如: cdda2wav).同时提供自动的MP3编码前端, 还可以自动地从internet上的光
盘数据库中查询光盘曲目. 如果协同DigitalDJ程序一起工作, 您可以创建自己的
"计算机化"的音乐库.

%description -l fr
Grip est un lecteur-extracteur de CD pour le Bureau GNome. Il a des
capacit閟 d'extraction du type cdparanoia incluses, mais il peut utiliser
un extracteur externe (comme cdda2wav). Il fournis 間alement une IHM
pour les encodeurs MP3, permettant simplement de prendre un disque
et de le transformer en MP3. La r閏up閞ation des noms sur Internet
permet 間alement d'avoir les informations sur les pistes aupr鑣 des 
serveurs CDDB.
Grip fonctionne 間alement avec DigitalDJ pour fournir une version unifi閑
informatique de votre discoth鑡ue.


%prep
rm -rf $RPM_BUILD_ROOT
%setup

%build
./configure --prefix=%{_prefix} --disable-shared-cdpar --disable-shared-id3
make

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT

make prefix=$RPM_BUILD_ROOT%{prefix} install

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-, root, root)
%doc README ChangeLog CREDITS COPYING INSTALL NEWS AUTHORS TODO ABOUT-NLS
%{_bindir}/grip
%{_datadir}/gnome/help/grip
%{_datadir}/gnome/apps/Multimedia/grip.desktop
%{_datadir}/pixmaps/gripicon.png
%{_datadir}/locale/*/LC_MESSAGES/grip.mo

%changelog
* Thu Apr 04 2002 Merlin Ma <merlin@turbolinux.com.cn>
- Added Simplified Chinese translations.
- Added locale dir and files in %files
- Modified spec file.

* Mon Jan 28 2002 Eric Lassauge <lassauge@mail.dotcom.fr>
- Added french translations
