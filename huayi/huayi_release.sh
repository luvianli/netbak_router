#!/bin/sh
#
#################################################
#### huayi release script           START
#### 
#################################################
basepath=$(cd `dirname $0`; pwd)

MAIL_LIST="gerry.li@hym2m.com rex@hym2m.com jerry.zhang@hym2m.com vicky.cheng@hym2m.com"

PRODUCT_MODEL=$1

SOFT_VERSION=

DATE=`date +%Y-%m-%d`

LOG_FILE=

FlashKey=$2

#### Functions
sendmail() {
	git remote -v >> ${basepath}/${LOG_FILE}
	echo -e "Thanks for ${USER}" >> ${basepath}/${LOG_FILE}
    echo -e "==> Sending build result email to ${MAIL_LIST}"  >> ${basepath}/${LOG_FILE}
	if [ MAIL_LIST ]
	then
		tail -n 50 ${basepath}/${LOG_FILE} > /tmp/${LOG_FILE}
		echo "Release:bulid $PRODUCT_MODEL version:$SOFT_VERSION complete!!!!!!" | 
		mailx -s "Build $PRODUCT_MODEL $SOFT_VERSION `date`" -a /tmp/${LOG_FILE} ${MAIL_LIST} >> /dev/null 2>&1
		#echo "Release:bulid $PRODUCT_MODEL version:$SOFT_VERSION complete!!!!!!" | mailx -v -s "Build `date`" -q /tmp/${LOG_FILE}.uu ${MAIL_LIST} >> /dev/null 2>&1
		rm -rf /tmp/${LOG_FILE}
	fi
}

#excute cmd git pull for update code
huayi_release_update() {
	git pull
}

#get software version by tag
huayi_release_version_control() {	
	SOFT_VERSION=`git for-each-ref --sort=taggerdate --format '%(refname)' refs/tags | grep $PRODUCT_MODEL | awk 'END {print}' | awk -F "-" '{print $NF}'`
	[ -z "$SOFT_VERSION" ] && {
		echo "no software version found !!!"
		echo "Plesae check $PRODUCT_MODEL software version !!!"
		echo "run git tag for detail information!!"
		exit 1
	}
	echo "==> SOFT_VERSION:$SOFT_VERSION"
	echo $SOFT_VERSION > $basepath/../package/base-files/files/etc/openwrt_version
	LOG_FILE="${PRODUCT_MODEL}_${SOFT_VERSION}_${DATE}.log"
}

huayi_release_build () {
	rm -rf ${basepath}/${LOG_FILE}
	echo -e "==> Start building $PRODUCT_MODEL $SOFT_VERSION"    	>> ${basepath}/${LOG_FILE}
	cd $basepath
	if [ "${FlashKey}" = "FlashKey=1" ]; then
		FlashId="FlashId=FlashEncryption"
	else
		FlashId="FlashId=None"
	fi
	make $PRODUCT_MODEL ${FlashKey}	 ${FlashId}			>> ${basepath}/${LOG_FILE} 2>&1	
    echo -e "==> Finished building $PRODUCT_MODEL $SOFT_VERSION"    	>> ${basepath}/${LOG_FILE}
}

#main start
echo "==> Start building $PRODUCT_MODEL"
huayi_release_update

[ -z "$1" ] && {
	echo "Usage: $0 [<release-option>...] <PRODUCT>"
	cd $basepath && make help
	exit 1
}

product_check=`cd $basepath && make help | grep $1`
[ -z "$product_check" ] && {
	echo "Usage: $0 [<release-option>...] <PRODUCT>"
	echo "Invaild PRODUCT!!!!!!!!!!!!!!!!!!!!!!!!!!!"
	exit 1
}

huayi_release_version_control

huayi_release_build

sendmail $PRODUCT_MODEL

#################################################
#### huayi release script           END
#################################################

