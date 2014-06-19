<?php
/**
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @author    OpenMediaVault Plugin Developers <plugins@omv-extras.org>
 * @copyright Copyright (c) 2009-2013 Volker Theile
 * @copyright Copyright (c) 2014 OpenMediaVault Plugin Developers
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
require_once("openmediavault/module.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/initscript.inc");

class OMVModuleWakealarm extends OMVModuleServiceAbstract
  implements OMVINotifyListener {
    /**
     * Get the module name.
     */
    public function getName() {
        return "wakealarm";
    }

    private function getPluginName() {
        return strtolower( $this->getName() );
    }

    private function getEventMessagePath() {
        $path = sprintf( "org.openmediavault.system.%s", $this->getPluginName() );
        return $path;
    }

    private function getEventMessagePathJob() {
        $path = sprintf( "%s.jobs.job", $this->getEventMessagePath() );
        return $path;
    }

    /**
     * Defines the modules that if present should start before the service
     * provided by this module.
     * @return An array of modules.
     */
    public function shouldStart() { return array("email"); }

    /**
     * Defines the modules that if present should be stopped after the
     * service provided by this module.
     * @return An array of modules.
     */
    function shouldStop() { return array("email"); }

    /**
     * Generate the configuration.
     * @throw E_EXEC_FAILED
     */
    public function applyConfig() {
        $cmd = "omv-mkconf wakealarm 2>&1";
        if(0 !== $this->exec($cmd, $output))
            throw new OMVException(OMVErrorMsg::E_EXEC_FAILED, $cmd, implode("\n", $output));
    }

    /**
     * Start the managed service.
     */
    public function startService() {
        // Changes to the cron files will be auto-detected by the cron
        // daemon via inotify by default, but in some situations it is
        // necessary to force a reload. According to the cron man-page,
        // it is only refreshed once an hour.
        /*$initd = new OMVSysVInitScript("wakealarm");
        $initd->invoke("reload");*/
    }

    /**
     * Bind listeners.
     */
    function bindListeners(OMVNotifyDispatcher $dispatcher) {
        $dispatcher->addListener(
          OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
          $this->getEventMessagePathJob(),
          array($this, "setDirty"));
    }
}

// Register module.
$moduleMgr = &OMVModuleMgr::getInstance();
$moduleMgr->registerModule(new OMVModuleWakealarm());
?>
