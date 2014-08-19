<?php
/**
 * @file
 * Drupal site audit fields.
 */

/**
 * Implements hook_drush_command().
 */
function site_audit_fields_drush_command() {
  $items = array();

  $items['audit_fields'] = array(
    'drush dependencies' => '',
    'description' => dt('Audit how fields are used in the site.'),
    'aliases' => array('auf'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
    'options' => site_audit_common_options(),
    'checks' => array(
      'FieldCount',
      'FieldUsage',
    ),
  );

  return $items;
}

/**
 * Audit modules and themes (extensions) for a given Drupal site.
 */
function drush_site_audit_fields_audit_fields() {
  require_once __DIR__ . '/site_audit_fields.site_audit.inc';
  $report = new SiteAuditReportFields();
  $report->render();
}

/**
 * Implements hook_drush_command_alter().
 */
function site_audit_fields_drush_command_alter(&$command) {
  if ($command['command'] == 'audit_all') {
    $command['reports'][] = array(
      'name' => 'Fields',
      'location' => __DIR__ . '/site_audit_fields.site_audit.inc',
    );
  }
}