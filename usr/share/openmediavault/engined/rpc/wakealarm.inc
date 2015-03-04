<?php
/**
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @author    OpenMediaVault Plugin Developers <plugins@omv-extras.org>
 * @copyright Copyright (c) 2009-2013 Volker Theile
 * @copyright Copyright (c) 2014-2015 OpenMediaVault Plugin Developers
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
require_once("openmediavault/object.inc");
require_once("openmediavault/config.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/util.inc");
require_once("openmediavault/rpcservice.inc");
require_once("openmediavault/notify.inc");

class OMVRpcServiceWakealarm extends OMVRpcServiceAbstract {
    /**
     * Get the RPC service name.
     */
    public function getName() {
        return "Wakealarm";
    }

    private function getPluginName() {
        return strtolower( $this->getName() );
    }

    private function getXPath() {
        $path = sprintf( "//system/%s", $this->getPluginName() );
        return $path;
    }

    private function getXPathJobs() {
        $path = sprintf( "%s/jobs", $this->getXPath() );
        return $path;
    }

    private function getXPathJob() {
        $path = sprintf( "%s/job", $this->getXPathJobs() );
        return $path;
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
     * Initialize the RPC service.
     */
    public function initialize() {
        $this->registerMethod("getJobs");
        $this->registerMethod("getJob");
        $this->registerMethod("setJob");
        $this->registerMethod("deleteJob");
    }

    /**
     * Get the cron jobs of the requested type.
     * @param params An array containing the following fields:
     *   \em start The index where to start.
     *   \em limit The number of objects to process.
     *   \em sortfield The name of the column used to sort.
     *   \em sortdir The sort direction, ASC or DESC.
     *   \em type Array of cron jobs types which are reboot, shutdown
     *   or userdefined.
     * @param context The context of the caller.
     * @return An array containing the requested objects. The field \em total
     *   contains the total number of objects, \em data contains the object
     *   array. An exception will be thrown in case of an error.
     * @throw E_CONFIG_GET_OBJECT_FAILED
     */
    public function getJobs($params, $context) {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array( "role" => OMV_ROLE_ADMINISTRATOR ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type"       : "object",
            "properties" : {
                "start"     : { "type" : "integer" },
                "limit"     : { '.$GLOBALS['OMV_JSONSCHEMA_COUNTFIELD'].' },
                "sortfield" : { '.$GLOBALS['OMV_JSONSCHEMA_SORTFIELD'].' },
                "sortdir"   : { '.$GLOBALS['OMV_JSONSCHEMA_SORTDIR'].' }
            }
        }');

        // Get configuration data.
        $objects = $xmlConfig->getList( $this->getXPathJob() );
        if(is_null($objects))
            throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED, $xpath);

        // Get cron jobs that match the given type.
        $jobs = array();
        foreach($objects as $objectk => $objectv) {
            // Modify result data.
            $objectv['enable']           = boolval($objectv['enable']);
            $objectv['everynminute']     = boolval($objectv['everynminute']);
            $objectv['everynhour']       = boolval($objectv['everynhour']);
            $objectv['everyndayofmonth'] = boolval($objectv['everyndayofmonth']);
            $jobs[] = $objectv;
        }

        // Filter the result.
        return $this->applyFilter($jobs, $params['start'], $params['limit'],
            $params['sortfield'], $params['sortdir']);
    }

    /**
     * Get a cron job configuration object.
     * @param params An array containing the following fields:
     *   \em uuid The UUID of the configuration object.
     * @param context The context of the caller.
     * @return The requested configuration object.
     * @throw E_CONFIG_GET_OBJECT_FAILED
     */
    public function getJob($params, $context) {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array( "role" => OMV_ROLE_ADMINISTRATOR ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type"       : "object",
            "properties" : {
                "uuid" : { '.$GLOBALS['OMV_JSONSCHEMA_UUID'].' }
            }
        }');

        // Get the configuration object.
        $xpath = sprintf("%s[uuid='%s']", $this->getXPathJob(), $params['uuid']);
        $object = $xmlConfig->get($xpath);
        if(is_null($object))
            throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED, $xpath);

        // Modify the result object.
        $object['enable']           = boolval($object['enable']);
        $object['everynminute']     = boolval($object['everynminute']);
        $object['everynhour']       = boolval($object['everynhour']);
        $object['everyndayofmonth'] = boolval($object['everyndayofmonth']);

        return $object;
    }

    /**
     * Set (add/update) a cron job.
     * @param params The method parameters.
     * @param context The context of the caller.
     * @return The stored configuration object.
     * @throw E_CONFIG_SET_OBJECT_FAILED
     */
    public function setJob($params, $context) {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array( "role" => OMV_ROLE_ADMINISTRATOR ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type"       : "object",
            "properties" : {
                "uuid"             : { '.$GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].' },
                "enable"           : { "type" : "boolean"},
                "minute"           : { "type" : "string", "format" : "regex", '.
                    '"pattern":"\/^[0-9]|1[0-9]|2[0-9]|3[0-9]|4[0-9]|5[0-9]|[*]$\/"},
                "everynminute"     : { "type" : "boolean" },
                "hour"             : { "type" : "string", "format" : "regex", '.
                    '"pattern":"\/^[0-9]|1[0-9]|2[0-3]|[*]$\/"},
                "everynhour"       : { "type" : "boolean"},
                "month"            : { "type" : "string", "format" : "regex", '.
                    '"pattern":"\/^[1-9]|1[0-2]|[*]$\/"},
                "dayofmonth"       : { "type" : "string", "format" : "regex", '.
                    '"pattern":"\/^[1-9]|1[0-9]|2[0-9]|3[0-1]|[*]$\/"},
                "everyndayofmonth" : { "type" : "boolean"},
                "dayofweek"        : { "type" : "string", "format" : "regex", '.
                    '"pattern":"\/^[1-7]|[*]$\/"},
                "comment"          : { "type" :"string"}
            }
        }');

        // Prepare configuration data.
        $object = array(
            "uuid"             => ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ? OMVUtil::uuid() : $params['uuid'],
            "enable"           => array_boolval($params, 'enable'),
            "minute"           => $params['minute'],
            "everynminute"     => array_boolval($params, 'everynminute'),
            "hour"             => $params['hour'],
            "everynhour"       => array_boolval($params, 'everynhour'),
            "dayofmonth"       => $params['dayofmonth'],
            "everyndayofmonth" => array_boolval($params, 'everyndayofmonth'),
            "month"            => $params['month'],
            "dayofweek"        => $params['dayofweek'],
            "comment"          => $params['comment']
        );

        // Set the configuration data.
        $success = FALSE;
        if($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
            // Append object to configuration.
            $success = $xmlConfig->set($this->getXPathJobs(), array("job" => $object));
        } else {
            // Update existing configuration object.
            $xpath = sprintf("%s[uuid='%s']", $this->getXPathJob(), $params['uuid']);
            $success = $xmlConfig->replace($xpath, $object);
        }

        if(FALSE === $success)
            throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);

        // Notify configuration changes.
        $dispatcher = &OMVNotifyDispatcher::getInstance();
        $dispatcher->notify(($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
            OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
            $this->getEventMessagePathJob(), $object);

        // Return the configuration object.
        return $object;
    }

    /**
     * Delete a cron job configuration object.
     * @param params An array containing the following fields:
     *   \em uuid The UUID of the configuration object.
     * @param context The context of the caller.
     * @return The deleted configuration object.
     */
    public function deleteJob($params, $context) {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array( "role" => OMV_ROLE_ADMINISTRATOR ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type"       : "object",
            "properties" : {
                "uuid" : { '.$GLOBALS['OMV_JSONSCHEMA_UUID'].' }
            }
        }');

        // Delete the configuration object.
        $xpath = sprintf("%s[uuid='%s']", $this->getXPathJob(), $params['uuid']);
        return $this->deleteConfigObjectByPath($xpath, $this->getEventMessagePathJob());
    }
}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceWakealarm());
