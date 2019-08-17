<?php

/**
 * @file
 * Features integration for 'contact' module.
 */

/**
 * Implements hook_features_api().
 */
function contact_features_api() {
  return array(
    'contact_categories' => array(
      'name' => t('Contact categories'),
      'feature_source' => TRUE,
      'default_hook' => 'contact_categories_defaults',
      'default_file' => FEATURES_DEFAULTS_INCLUDED,
    ),
  );
}

/**
 * Implements hook_features_export_options() for 'contact_categories' component.
 *
 * @return string[]
 */
function contact_categories_features_export_options() {
  $options = array();
  /** @noinspection NullPointerExceptionInspection */
  $categories = db_select('contact', 'c')->fields('c')->execute()->fetchAll();
  foreach ($categories as $category) {
    $options[(string) $category->category] = (string) $category->category;
  }
  return $options;
}

/**
 * Implements hook_features_export() for 'contact_categories' component.
 *
 * @param array $data
 * @param array $export
 * @param string $module_name
 *
 * @return array
 */
function contact_categories_features_export($data, &$export, $module_name = '') {
  $export['dependencies']['features'] = 'features';
  $export['dependencies']['contact'] = 'contact';

  foreach ($data as $name) {
    $export['features']['contact_categories'][$name] = $name;
  }

  return array();
}

/**
 * Implements hook_features_export_render() for 'contact_categories' component.
 *
 * @param string $module
 * @param string[] $data
 * @param array|null $export
 *
 * @return string[]
 *   Format: $[$hook] = $function_body
 */
function contact_categories_features_export_render($module, $data, $export = NULL) {
  $render = array();
  foreach ($data as $name) {
    /** @noinspection NullPointerExceptionInspection */
    $export_category = db_select('contact', 'c')
      ->fields('c', array('cid', 'category'))
      ->condition('category', $name, 'LIKE')
      ->execute()
      ->fetchAll();
    if (isset($export_category[0]->cid) && ($category = contact_load($export_category[0]->cid))) {
      unset($category['cid']);
      $render[$name] = $category;
    }
  }
  return array('contact_categories_defaults' => '  return ' . features_var_export($render, '  ') . ';');
}

/**
 * Implements hook_features_revert() for 'contact_categories' component.
 *
 * @param string $module
 *
 * @throws \InvalidMergeQueryException
 */
function contact_categories_features_revert($module) {
  contact_categories_features_rebuild($module);
}

/**
 * Implements hook_features_rebuild() for 'contact_categories' component.
 *
 * @param string $module
 *
 * @throws \InvalidMergeQueryException
 */
function contact_categories_features_rebuild($module) {
  if ($defaults = features_get_default('contact_categories', $module)) {
    foreach ($defaults as $default_category) {
      $existing_categories = db_select('contact', 'c')
        ->fields('c', array('cid', 'category'))
        ->execute()
        ->fetchAll();
      if ($existing_categories) {
        foreach ($existing_categories as $existing_category) {
          if ($default_category['category'] == $existing_category->category) {
            db_update('contact')
              ->fields(
                array(
                  'recipients' => $default_category['recipients'],
                  'reply' => $default_category['reply'],
                  'weight' => $default_category['weight'],
                  'selected' => $default_category['selected'],
                )
              )
              ->condition('category', $existing_category->category, '=')
              ->execute();
          }
          else {
            db_merge('contact')
              ->key(array('category' => $default_category['category']))
              ->fields($default_category)
              ->execute();
          }
        }
      }
      else {
        db_merge('contact')
          ->key(array('category' => $default_category['category']))
          ->fields($default_category)
          ->execute();
      }
    }
  }
}
