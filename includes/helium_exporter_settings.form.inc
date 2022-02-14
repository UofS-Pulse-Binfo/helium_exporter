<?php

/**
 * @file
 * The main settings form page of this module.
 */

/**
 * Menu callback.
 * Implements hook_form().
 */
function helium_exporter_settings_form($form, &$form_state) {
  $module = drupal_get_path('module', 'helium_exporter');
  $path = $GLOBALS['base_url'] . '/' . $module;
  drupal_add_css($module . '/theme/css/style-helium-exporter-settings.css');

  // Get all relationship types, becomes select option in the following fields.
  $relations = helium_exporter_get_relationship_types();
  $form_state['relationships'] = $relations;

  // Images of subject/object.  
  $pedigree_img_subject = $path . '/theme/images/pedigree-subject.gif';
  $pedigree_img_object = $path . '/theme/images/pedigree-object.gif';
  
  // Inspect configuration and instruct admin to set a value when
  // this is still set to 0 (0 by default).
  $default_rels = variable_get('helium_exporter_pedigree_relations');
  $default_rels = unserialize($default_rels);

  if ($default_rels == 0) {
    drupal_set_message('Please set all configuration for this module.', 'warning');
  }


  // FIELD: FIELDSET download data.
  $form['configure'] = array(
    '#type' => 'fieldset',
    '#title' => t('Stock Pedigree Relationship'),
    '#id' => 'helium-exporter-settings-fieldset',
    '#description' => "To ensure that pedigrees display correctly, you need to
      configure this module to use your vocabulary correctly. Specifically,
      <ol>
        <li>Ensure that you've choosen all the relationships needed to get from
          your child stock/germplasm to it's parents.</li>
        <li>Ensure that relationships will be followed in the right direction.
          Since relationships are bi-directional in Tripal/Chado you need to
          configure which relationships have the parent as the subject and
          which have the parent as the object.</li>
        <li>Ensure to indicate terms used to identify a maternal parent and a paternal parent.</li>
      </ol>
      <em><strong>NOTE:</strong> Each relationship can only be followed in ONE
        direction.</em>"
  );

  
  // Preselect default configuration for subject and object using 
  // system variable.
  // NOTE: subject label on object field and object label on subject field.

  // SUBJECT:
  $form['configure']['pedigree_subject']['container'] = array(
    '#prefix' => '<div class="helium-exporter-configuration">',
    '#suffix' => '</div>'
  );

  $form['configure']['pedigree_subject']['container']['image'] = array(
    '#markup' => '<div class="helium-exporter-settings-diagram">
      <img src="' . $pedigree_img_subject  . '" heigth="155" width="280" />
    </div>'
  );

  $form['configure']['pedigree_subject']['container']['field_object'] = array(
    '#type' => 'select',
    '#title' => t('Subject is Parent'),
    '#options' => $relations,
    '#multiple' => TRUE,
    '#default_value' => array_keys($default_rels['object']),
    '#description' => 'Select the relationships below where the parent-side of the relationship 
      is the subject. This will put the subject higher in the tree than the 
      object, as shown in the digram above.',
    '#prefix' => '<div class="helium-exporter-settings-field">',
    '#suffix' => '</div><div class="helium-exporter-clear-float"></div>'
  );


  // OBJECT:
  $form['configure']['pedigree_object']['container'] = array(
    '#prefix' => '<div class="helium-exporter-configuration">',
    '#suffix' => '</div>'
  );

  $form['configure']['pedigree_object']['container']['image'] = array(
    '#markup' => '<div class="helium-exporter-settings-diagram">
      <img src="' . $pedigree_img_object  . '" heigth="155" width="280" />
    </div>'
  );

  $form['configure']['pedigree_object']['container']['field_subject'] = array(
    '#type' => 'select',
    '#title' => t('Object is Parent'),
    '#options' => $relations,
    '#multiple' => TRUE,
    '#default_value' => array_keys($default_rels['subject']),
    '#description' => 'Select the relationships below where the parent-side of the relationship 
      is the object. This will put the object higher in the tree than the 
      subject, as shown in the digram above.',
    '#prefix' => '<div class="helium-exporter-settings-field">',
    '#suffix' => '</div><div class="helium-exporter-clear-float"></div>'
  );

  // Terms used.
  // Tell us which term is used for maternal parent and which term is used
  // for paternal parent.
  $form['configure']['terms_parents']['container'] = array(
    '#prefix' => '<div class="helium-exporter-configuration">',
    '#suffix' => '</div>'
  );

  $form['configure']['terms_parents']['container']['field_term_maternal'] = array(
    '#type' => 'select',
    '#title' => t('Select term used to identify maternal parent'),
    '#options' => $relations,
    '#default_value' => $default_rels['maternal'],
  );

  $form['configure']['terms_parents']['container']['field_term_paternal'] = array(
    '#type' => 'select',
    '#title' => t('Select term used to identify paternal parent'),
    '#options' => $relations,
    '#default_value' => $default_rels['paternal'],
  );


  $form['configure']['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Save Configuration'
  );

  return $form;
}

/**
 * Implements hook_form_validate().
 */
function helium_exporter_settings_form_validate($form, $form_state) {
  if ($form_state['values']['field_subject'] && $form_state['values']['field_object']) {
    // Check that no relationship exists in both subject & object lists.
    // Trying to avoid users configuring endless loops :)
    $common = array_intersect($form_state['values']['field_subject'], $form_state['values']['field_object']);
    if (!empty($common)) {

      // Make a more useful error message
      $common_rels = array();
      foreach ($common as $id) {
        $common_rels[] = $form_state['relationships'][$id];
      }

      // Provide error message.
      form_set_error('subject_rels', 'Each relationship can only be followed in ONE direction (ie: be selected in one of the "Relationships to include" lists)');
      form_set_error('object_rels', 'Relationships selected in both lists include: <em>' . implode(', ', $common_rels) . '</em>.');
    }
  }
  else {
    // Nothing selected for subject and object.
    if (empty($form_state['values']['field_subject'])) {
      form_set_error('subject_rels', 'No value selected for Subject is Parent field.');    
    }
    
    if (empty($form_state['values']['field_object'])) {
      form_set_error('object_rels', 'No value selected for Object is Parent field.');    
    }
  }  

  // Maternal and paternal parents term used must not be the same.
  if ($form_state['values']['field_term_maternal'] == $form_state['values']['field_term_paternal']) {
    form_set_error('field_term_maternal', 'The terms used to identify maternal and paternal must not be the same.');
  }
}

/**
 * Implements hook_form_submit().
 */
function helium_exporter_settings_form_submit($form, $form_state) {
  if ($form_state['values']['field_subject'] && $form_state['values']['field_object']
     && $form_state['values']['field_term_maternal'] && $form_state['values']['field_term_paternal']) {
    // Use form values set by administrator.
    $object_rels = array_intersect_key($form_state['relationships'], $form_state['values']['field_object']);
    $subject_rels = array_intersect_key($form_state['relationships'], $form_state['values']['field_subject']);

    $rels = array(
      'maternal' => $form_state['values']['field_term_maternal'],
      'paternal' => $form_state['values']['field_term_paternal'],
      'object' => $object_rels,
      'subject' => $subject_rels
    );
  
    variable_set('helium_exporter_pedigree_relations', serialize($rels));
  }
  else {
    variable_set('helium_exporter_pedigree_relations', 0);
  }
}