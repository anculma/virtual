<?php

/**
 * @file
 * API payment pagosonline
 */

/**
 * Page response.
 */
function commerce_pagosonline_response($order) {
  $param = commerce_pagosonline_get_param($_GET);

  if ($param['estado_pol'] == 5) {
    commerce_payment_redirect_pane_previous_page($order);
    drupal_set_message(t('Order Canceled'), 'error');
    drupal_goto(commerce_checkout_order_uri($order));
  }
  else {
    drupal_set_message(t('Thanks for your purchase. We are awaiting confirmation from PagosOnline. Once the payment has been aproved, you will get more information about your order.'));
    drupal_goto('<front>');
  }
}

/**
 * Page confirmation.
 */
function commerce_pagosonline_confirmation($order) {
  $param = commerce_pagosonline_get_param($_POST);
  $transaction = array();
  if ($param['estado_pol'] == 5) {
    commerce_payment_redirect_pane_previous_page($order);
  }
  else {
    $transaction = commerce_pagosonline_save_transation($param, $order);
    if ($transaction->status == COMMERCE_PAYMENT_STATUS_SUCCESS) {
      commerce_payment_redirect_pane_next_page($order);
    }
  }
  module_invoke_all('commerce_pagosonline_confirmation', $order, $transaction);
  $result = array(
    'estate' => TRUE,
  );
  drupal_json_output($result);
}

/**
 * Save transaction.
 */
function commerce_pagosonline_save_transation($param, $order) {
  $result = db_insert('commerce_pagosonline')
    ->fields(array(
      'order_id' => $order->order_id,
      'created' => REQUEST_TIME,
      'reference_pagosonline' => $param['ref_pol'],
      'state_transaction' => $param['estado_pol'],
      'value' => $param['valor'],
      'response' => serialize($param),
    ))
    ->execute();

  if ($param['cus'] == 'null') {
    $param['cus'] = 0;
  }

  $path_transation = 'pagosonline/transaction/view/' . $result . '/' . md5($result . '~' . 'VIEW-TRANSATION');

  $transaction = commerce_payment_transaction_new('pagosonline', $order->order_id);

  $transaction->payload[REQUEST_TIME] = $param;

  $message = array();

  $transaction->remote_id = $param['transaccion_id'];

  $txt_state = commerce_pagosonline_transaction_state($param['estado_pol']);
  $txt_reponse = commerce_pagosonline_response_code($param['codigo_respuesta_pol']);

  $transaction->remote_status = $txt_state . ' - ' . $txt_reponse;

  $message[] = $param['estado_pol'] . ': ' . $txt_state;
  $message[] = $param['codigo_respuesta_pol'] . ': ' . $txt_reponse;
  $message[] = l(t('View transaction'), $path_transation);


  switch ($param['estado_pol']) {
    case 4:
      $transaction->status = COMMERCE_PAYMENT_STATUS_SUCCESS;
      break;

    case 7:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
      $transaction->status = COMMERCE_PAYMENT_STATUS_PENDING;
      break;

    default:
      $transaction->status = COMMERCE_PAYMENT_STATUS_FAILURE;
  }

  $transaction->instance_id = $param['extra1'];
  $exchange_money = $param['valor'] / $param['tasa_cambio'];
  $amount = commerce_currency_decimal_to_amount($exchange_money, $param['moneda']);
  $transaction->amount = $amount;
  $transaction->currency_code = $param['moneda'];
  $transaction->message = implode('<br />', $message);
  commerce_payment_transaction_save($transaction);

  return $transaction;
}

/**
 * Load parameters.
 */
function commerce_pagosonline_get_param($target) {
  $param = array();
  if (is_array($target)) {
    foreach ($target as $key => $value) {
      $param[$key] = check_plain($target[$key]);
    }
  }
  return $param;
}

/**
 * Code reponse, transaction.
 */
function commerce_pagosonline_response_code($key = NULL) {
  $code = array(
    1 => t('Transaction approved'),
    2 => t('Payment canceled by the user'),
    3 => t('Payment canceled by the user while validating'),
    4 => t('Transaction rejected by the entity'),
    5 => t('Transaction declined by the entity'),
    6 => t('Insufficient funds'),
    7 => t('Invalid card'),
    8 => t('Go to your bank'),
    9 => t('Expired card'),
    10 => t('Restricted card'),
    11 => t('POL Discretionary'),
    12 => t('Expiration date or security code invalid'),
    13 => t('Repit transaction'),
    14 => t('Invalid transaction'),
    15 => t('Transaction in validating process'),
    16 => t('Invalid user-password combination'),
    17 => t('Amount exceeds maximum allowed by the entity'),
    18 => t('Invalid identification document'),
    19 => t('Paid Transaction, capturing data'),
    20 => t('Abandoned transaction'),
    21 => t('Impossible to reverse transaction'),
    22 => t('Card not allowed to do payments by internet'),
    23 => t('Rejected transaction'),
    24 => t('Partial transaction approved'),
    25 => t('Rejected no-confirmation'),
    26 => t('Voucher generated, waiting for bank payment'),
    9994 => t('Transaction pending to be confirmed'),
    9995 => t('Digital certificate not found'),
    9996 => t("Entity doesn't answer"),
    9997 => t('Messaging error with the bank'),
    9998 => t('Error in bank'),
    9999 => t('Not specified error'),
  );
  return $key ? $code[$key] : $code;
}

/**
 * Return state transaction.
 */
function commerce_pagosonline_transaction_state($key = NULL) {
  $state = array(
    1 => t('Unopened'),
    2 => t('Opened'),
    4 => t('Paid'),
    5 => t('Canceled'),
    6 => t('Rejected'),
    7 => t('Validating'),
    8 => t('Reversed'),
    9 => t('Fraudulent reversed'),
    10 => t('Sent to financial institution'),
    11 => t('Capturing credit card data'),
    12 => t('Awaiting PSE system confirmation'),
    13 => t('Active ACH Debits'),
    14 => t('Confirming Efecty payment'),
    15 => t('Printed'),
    16 => t('ACH debit registered'),
  );
  return $key ? $state[$key] : $state;
}

/**
 * Return language interface pagosonline.
 */
function commerce_pagosonline_language($key = NULL) {
  $languages = array(
    'fr' => t('French'),
    'en' => t('English'),
    'es' => t('Spanish'),
    'it' => t('Italian'),
  );
  return $key ? $languages[$key] : $languages;
}

/**
 * Return currencies, avaliable.
 */
function commerce_pagosonline_currencies($key = NULL) {
  $currencies = array(
    'COP' => t('Colombian peso'),
    'EUR' => t('Euro'),
    'GBP' => t('Pound sterling'),
    'MXN' => t('Mexican peso'),
    'USD' => t('United States dollar'),
    'VEB' => t('Venezuelan bolivar fuerte'),
  );
  return $key ? $currencies[$key] : $currencies;
}

/**
 * Form payment.
 */
function commerce_pagosonline_payment_form($key = NULL) {
  $payment_form = array(
    10 => t('VISA'),
    11 => t('MASTERCARD'),
    12 => t('AMEX'),
    22 => t('DINERS'),
    24 => t('Verified by VISA'),
    25 => t('PSE'),
    27 => t('VISA debit'),
    30 => t('Efecty'),
    31 => t('Referenced payment'),
  );
  return $key ? $payment_form[$key] : $payment_form;
}

/**
 * Type form payment.
 */
function commerce_pagosonline_payment_type($key = NULL) {
  $payment_type = array(
    2 => t('Credit Card'),
    3 => t('Verified by VISA'),
    4 => t('PSE (Account corriente/ahorros.)'),
    5 => t('Debit ACH'),
    7 => t('Cash Payment (Efecty)'),
    8 => t('Referenced payment'),
  );
  return $key ? $payment_type[$key] : $payment_type;
}

/**
 * Digital signature of the transaction.
 */
function commerce_pagosonline_get_firm($settings, $state_transation = "") {
  $params = array(
    $settings['po_encryption_key'],
    $settings['po_uid'],
    $settings['sale_reference'],
    $settings['amount'],
    $settings['currency_code'],
  );
  if (!empty($state_transation)) {
    $params[] = $state_transation;
  }
  return md5(implode('~', $params));
}

/**
 * Generate number reference unique.
 */
function commerce_pagosonline_get_reference($alias, $order_id) {
  return $alias . $order_id;
}

/**
 * Generate url response and confirmation.
 */
function commerce_pagosonline_get_url($order_id) {
  $token = uniqid();

  $url_response = COMMERCE_PAGOSONLINE_URL_RESPONSE . '/' . $order_id . '/' . $token . '/' . commerce_pagosonline_get_md5($order_id, $token, 'RESPONSE');
  $url_confirmation = COMMERCE_PAGOSONLINE_URL_CONFIRMATION . '/' . $order_id . '/' . $token . '/' . commerce_pagosonline_get_md5($order_id, $token);

  return array(
    'response' => url($url_response, array(
      'absolute' => TRUE,
    )),
    'confirmation' => url($url_confirmation, array(
      'absolute' => TRUE,
    )),
  );
}

/**
 * Encrypted key.
 */
function commerce_pagosonline_get_md5($order_id, $token, $type = 'CONFIRMATION') {
  return md5($order_id . '~' . strtoupper($type) . '~' . $token);
}
