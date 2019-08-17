<?php

/**
 * Implements hook_features_export() for 'context' component.
 *
 * @param string[] $data
 * @param array $export
 * @param string $module_name
 *
 * @return array
 */
function context_features_export($data, &$export, $module_name = '') {
  $pipe = ctools_component_features_export('context', $data, $export, $module_name);

  $contexts = context_load();
  foreach ($data as $identifier) {
    if (isset($contexts[$identifier])) {
      $context = $contexts[$identifier];
      // Conditions.
      // Currently only node and views conditions are supported.
      // @TODO: Should this be delegated to a method on the plugin?
      foreach (array('node', 'views') as $key) {
        if (!empty($context->conditions{$key}['values'])) {
          foreach ($context->conditions{$key}['values'] as $item) {
            // Special pipe for views
            if ($key === 'views') {
              $split = explode(':', $item);
              $view_name = array_shift($split);
              $pipe[$key][$view_name] = $view_name;
            }
            else {
              $pipe[$key][$item] = $item;
            }
          }
        }
      }
      // Reactions.
      if (!empty($context->reactions['block']['blocks'])) {
        foreach ($context->reactions['block']['blocks'] as $block) {
          $block = (array) $block;
          $bid = "{$block['module']}-{$block['delta']}";
          $pipe['block'][$bid] = $bid;
        }
      }
    }
  }
  return $pipe;
}

/**
 * Implements hook_features_revert() for 'context' component.
 *
 * @param string $module
 *   Name of module to revert content for
 */
function context_features_revert($module = NULL) {
  ctools_component_features_revert('context', $module);
  context_invalidate_cache();
}
