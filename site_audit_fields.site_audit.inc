<?php

class SiteAuditReportFields extends SiteAuditReportAbstract {
  /**
   * Implements \SiteAudit\Report\Abstract\getLabel().
   */
  public function getLabel() {
    return dt('Fields');
  }
}
abstract class SiteAuditFieldsSiteAuditCheckAbstract extends SiteAuditCheckAbstract {
  protected $module = 'site_audit_fields';
  protected $check;
  protected $label;
  protected $description;

  /**
   * Implements \SiteAudit\Check\Abstract\getLabel().
   */
  public function getLabel() {
    return dt($this->label);
  }

  /**
   * Implements \SiteAudit\Check\Abstract\getDescription().
   */
  public function getDescription() {
    return dt($this->description);
  }

  /**
   * Implements \SiteAudit\Check\Abstract\getResultFail().
   */
  public function getResultFail() {
    // format the result of the failure.
    return $this->formatResult('failure');
  }

  /**
   * Format the output for the test result.
   */
  protected function formatResult($result_type) {
    // Get a sipmle string output.
    $output = !empty($this->registry[$this->module][$this->check][$result_type])
      ? $this->registry[$this->module][$this->check][$result_type] : '';

    if (isset($this->registry[$this->module][$this->check]['value'])) {
      if (is_array($this->registry[$this->module][$this->check]['value'])) {
        $values = $this->registry[$this->module][$this->check]['value'];
        if (drush_get_option('html')) {
          $output .= '<ul>';
          foreach ($values as $value) {
            $output .= '<li>' . $value . '</li>';
          }
          $output .= '</ul>';
        }
        else {
          foreach ($values as $value) {
            if (!drush_get_option('json')) {
              $output .= PHP_EOL . str_repeat(' ', 6);
            }
            $output .= '- ' . $value;
          }
          if (!drush_get_option('json')) {
            $output .= str_repeat(' ', 6);
          }
        }
      }
      else {
        $output .= ' Additional: "' . $this->registry[$this->module][$this->check]['value'] . '"';
      }
    }
    return $output;
  }
  /**
   * Implements \SiteAudit\Check\Abstract\getResultInfo().
   */
  public function getResultInfo() {
    return $this->formatResult('info');
  }

  /**
   * Implements \SiteAudit\Check\Abstract\getResultPass().
   */
  public function getResultPass() {
    return $this->formatResult('pass');
  }

  /**
   * Implements \SiteAudit\Check\Abstract\getResultWarn().
   */
  public function getResultWarn() {
    return $this->formatResult('warn');
  }

  /**
   * Implements \SiteAudit\Check\Abstract\getAction().
   */
  public function getAction() {}

  /**
   * Implements \SiteAudit\Check\Abstract\calculateScore().
   */
  public function calculateScore() {
    return SiteAuditCheckAbstract::AUDIT_CHECK_SCORE_FAIL;
  }
}

class SiteAuditCheckFieldsFieldCount extends SiteAuditFieldsSiteAuditCheckAbstract {
  protected $fieldCount = FALSE;
  protected $label = 'Field Count';
  protected $description = 'Counts the number of fields used in a site.';
  protected $check = 'field_count';

  /**
   * Implements \SiteAudit\Check\Abstract\calculateScore().
   */
  public function calculateScore() {

    if (!module_exists('field')) {
      $this->abort = TRUE;
      $this->registry[$this->module][$this->check]['info'] = dt('The field module is not enabled.');
      return SiteAuditCheckAbstract::AUDIT_CHECK_SCORE_INFO;
    }
    $all_fields = field_info_field_map();
    $this->fieldCount = count($all_fields);

    if ($this->fieldCount > 75) {
      $this->registry[$this->module][$this->check]['warn'] = dt("There are a total of {$this->fieldCount} fields created on this site.");
      return SiteAuditCheckAbstract::AUDIT_CHECK_SCORE_WARN;
    }
    $this->registry[$this->module][$this->check]['pass'] = dt("There are a total of {$this->fieldCount} fields created on this site.");
    return SiteAuditCheckAbstract::AUDIT_CHECK_SCORE_PASS;
  }
}

/**
 * Class SiteAuditCheckFieldsFieldUsage
 *
 * @todo add field type
 */
class SiteAuditCheckFieldsFieldUsage extends SiteAuditFieldsSiteAuditCheckAbstract {
  protected $fieldCount = FALSE;
  protected $label = 'Field Usage';
  protected $description = 'Gets information about how fields are used in entities.';
  protected $check = 'field_usage';

  /**
   * Implements \SiteAudit\Check\Abstract\calculateScore().
   */
  public function calculateScore() {

    if (!module_exists('field')) {
      $this->abort = TRUE;
      $this->registry[$this->module][$this->check]['info'] = dt('The field module is not enabled.');
      return SiteAuditCheckAbstract::AUDIT_CHECK_SCORE_INFO;
    }

    // Get field Usage Info.
    $this->registry[$this->module][$this->check]['info'] = $this->getFieldUsageInfo();
    return SiteAuditCheckAbstract::AUDIT_CHECK_SCORE_INFO;
  }

  /**
   * Creates a report on how fields are used in a site.
   */
  protected function getFieldUsageInfo() {

    $all_fields = field_info_field_map();

    $field_usage = array();

    foreach ($all_fields as $field_name=>$field) {
      foreach ($field['bundles'] as $entity_type=>$bundles) {
        foreach ($bundles as $bundle_name) {

          // Get the actual number of fields used.
          $query = new EntityFieldQuery();
          $query->entityCondition('entity_type', $entity_type)
            ->entityCondition('bundle', $bundle_name)
            ->fieldOrderBy($field_name, 'delta', 'ASC')
            ->count();
          $field_count = $query->execute();

          // Get the number of fields stored for this bundle.
          $field_usage[$entity_type][$bundle_name][$field_name] = is_numeric($field_count) ? $field_count : 'n/a';
        }
      }
    }

    $nl = chr(10);

    // First sort entity types by name.
    ksort($field_usage);

    foreach ($field_usage as $entity_type=>&$bundle_names) {
      // Sort bundles by name.
      ksort($bundle_names);

      $output.=     "{$entity_type}{$nl}";
      $output.=     "=============={$nl}";
      $output.=     "| Field Name            | Field Type            | Usage Count      | {$nl}";
      $output.=     "| :-------------------: | --------------------- | ---------------- | {$nl}";
      foreach ($bundle_names as $bundle_name=>&$field_names) {

        // Sort fields by name.
        ksort($field_names);

        // Get a node count for this entity.
        $node_count = $this->getNodeCount($entity_type, $bundle_name);

        $node_count = $this->getNodeCount($entity_type, $bundle_name);

        $output.=   "|+g **{$bundle_name}**  |+g                     |+g  {$node_count} | {$nl}";
        foreach ($field_names as $field_name=>$count) {
          // Get the field type fro this field.
          $field_info = field_info_field($field_name);

          $output.= "| {$field_name}         | {$field_info['type']} | {$count}         | {$nl}";
        }
        $output.=   "{$nl}{$nl}";
        $output.= "{$nl}";
      }
    }

    if (drush_get_option('html')) {

      // Load up a markdown parser.
      require_once 'vendor/autoload.php';
      $Parsedown = new Parsedown();

      // Process markdown and add bootstrap classes.
      $output = str_replace('<table>', '<table class="table table-bordered table-striped" style="table-layout: fixed;">',$Parsedown->text($output));
      $output = preg_replace('/<td.*>\+g/i', '<td class="success">', $output);
    }

    // Return printables.
    return $output;
  }

  /**
   * Creates a report on how fields are used in a site.
   */
  protected function getNodeCount($entity_type, $bundle_name) {
    if ($entity_type == 'node') {
      $node_count = db_query("SELECT COUNT(*) FROM {node} n WHERE n.type = :type", array(':type' => $bundle_name))->fetchField();
      return "{$node_count} node(s)";
    } else {
      return '';
    }
  }
}
