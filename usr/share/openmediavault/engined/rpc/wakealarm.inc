<?php
/**
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @author    OpenMediaVault Plugin Developers <plugins@omv-extras.org>
 * @copyright Copyright (c) 2009-2013 Volker Theile
 * @copyright Copyright (c) 2014-2025 openmediavault plugin developers
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

class OMVRpcServiceWakealarm extends \OMV\Rpc\ServiceAbstract
{
    public function getName()
    {
        return "Wakealarm";
    }

    public function initialize()
    {
        $this->registerMethod("getJobList");
        $this->registerMethod("getJob");
        $this->registerMethod("setJob");
        $this->registerMethod("deleteJob");
    }

    public function getJobList($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ['role' => OMV_ROLE_ADMINISTRATOR]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, 'rpc.common.getlist');
        // Get the list of configuration objects.
        $db = \OMV\Config\Database::getInstance();
        $objects = $db->getAssoc('conf.system.wakealarm.job');
        // Filter the result objects.
        return $this->applyFilter($objects, $params['start'], $params['limit'],
            $params['sortfield'], $params['sortdir']);
    }

    public function getJob($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, "rpc.common.objectuuid");
        // Get the configuration object.
        $db = \OMV\Config\Database::getInstance();
        return $db->getAssoc("conf.system.wakealarm.job", $params['uuid']);
    }

    public function setJob($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, "rpc.wakealarm.setjob");
        // Prepare the configuration object.
        $object = new \OMV\Config\ConfigObject("conf.system.wakealarm.job");
        $object->setAssoc($params);
        // Set the configuration object.
        $db = \OMV\Config\Database::getInstance();
        $db->set($object);
        // Return the configuration object.
        return $object->getAssoc();
    }

    public function deleteJob($params, $context)
    {
       // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, "rpc.common.objectuuid");
        // Delete the configuration object.
        $db = \OMV\Config\Database::getInstance();
        $object = $db->get("conf.system.wakealarm.job", $params['uuid']);
        $db->delete($object);
        // Return the deleted configuration object.
        return $object->getAssoc();
    }
}
