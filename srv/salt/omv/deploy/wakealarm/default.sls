# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    OpenMediaVault Plugin Developers <plugins@omv-extras.org>
# @copyright Copyright (c) 2019-2025 openmediavault plugin developers
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

{% set config = salt['omv_conf.get']('conf.system.wakealarm') %}

configure_wakealarm:
  file.managed:
    - name: "/etc/wakealarm.conf"
    - source:
      - salt://{{ tpldir }}/files/etc-wakealarm_conf.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - user: root
    - group: root
    - mode: 644

configure_wakealarm_logrotate:
  file.managed:
    - name: "/etc/logrotate.d/wakealarm"
    - contents: |
        /var/log/wakealarm.log {
            rotate 4
            weekly
            compress
            missingok
            notifempty
        }
    - user: root
    - group: root
    - mode: 644
