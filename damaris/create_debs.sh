export SPC_SOURCE=/home/markusro/sources/spc_driver/micx_drv
export DEBEMAIL="Markus.Rosenstihl@physik.tu-darmstadt.de"
export DEBFULLNAME="Markus Rosenstihl"
#dch -i
head=$(pwd)
# create backend deb
dpkg-buildpackage -us -uc
# create frontend deb
cd frontends/python-damaris
dpkg-buildpackage -us -uc
cd -
# extract revision
rev=$(svn info|grep Revision|cut -d" " -f2)
newdir="$head/../dpkg-archive/$(date +%F)/$rev/"
mkdir -p $newdir
for deb in $(find $(pwd)/.. -regex ".*\(deb\|changes\|dsc\|tar\.gz\)" |grep -v archive)
do
	mv -v "$deb" "$newdir"
done

#ssh element sh /home/markusro/public_html/prepare.sh
scp -r $newdir  element:/home/markusro/public_html/damaris-nightly


