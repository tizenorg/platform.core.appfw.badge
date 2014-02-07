#!/bin/sh

source /etc/tizen-platform.conf

sqlite3 $TZ_USER_DB/.badge.db < $TZ_SYS_SHARE/badge/ressources/badge.sql
chmod 600 $TZ_USER_DB/.badge.db
