<?php

/**
 * @file
 * Transaction page
 */

/**
 * Page transaction view
 */
function commerce_pagosonline_transaction_view($id_transaction) {
  $result = db_select('commerce_pagosonline', 'p')
    ->fields('p')
    ->condition('id', $id_transaction)
    ->execute()
    ->fetchAssoc();
  if ($result) {
    $content = array();
    module_load_include('inc', 'commerce_pagosonline');
    $data = array();

    foreach ($result as $key => $value) {
      switch ($key) {
        case 'order_id':
          $value = l($value, 'admin/commerce/orders/' . $value);
          break;

        case 'created':
          $value = date('Y-m-d H:i:s', $value);
          break;

        case 'response':
          $value = '<pre>' . print_r(unserialize($value), TRUE) . '</pre>';
          break;

        case 'state_transaction':
          $value = commerce_pagosonline_transaction_state($value);
          break;
      }
      $data[] = array($key, $value);
    }
    $content['transation'] = array(
      '#prefix' => '<div>',
      'suffix' => '</div>',
    );

    $content['transation']['detail'] = array(
      '#markup' => theme('table', array(
        'header' => array(
          'Name',
          'Value',
        ),
        'rows' => $data,
      )),
    );

    $path = 'admin/commerce/orders/' . $result['order_id'] . '/payment';
    $content['order'] = array(
      '#prefix' => '<div>',
      'suffix' => '</div>',
      '#markup' => l(t('View Payments'), $path),
    );

    return $content;
  }
  else {
    return MENU_NOT_FOUND;
  }
}
