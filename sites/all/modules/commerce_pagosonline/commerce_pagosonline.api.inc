<?php

/**
 * @file
 * Hooks provided by the PagosOnline module.
 */

/**
 * Allows modules to react to the confirmation of transaction.
 *
 * @param $order
 *   The order object.
 *
 * @param transaction
 *   The transaction object.
 */
function hook_commerce_pagosonline_confirmation($order, $transaction) {
  // No example.
}