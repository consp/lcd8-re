package com.example.bikenavunit;

import static com.example.bikenavunit.navlistener.NavigationListenerEmitterKt.NAVIGATION_DATA;
import static com.example.bikenavunit.navlistener.NavigationListenerEmitterKt.NAVIGATION_DATA_UPDATED;
import static com.example.bikenavunit.navlistener.NavigationListenerEmitterKt.NAVIGATION_STARTED;
import static com.example.bikenavunit.navlistener.NavigationListenerEmitterKt.NAVIGATION_STOPPED;
import static com.example.bikenavunit.navlistener.NavigationListenerEmitterKt.NOTIFICATIONS_ACCESS_RESULT;

import android.Manifest;
import android.annotation.SuppressLint;
import android.app.PendingIntent;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;

import androidx.activity.EdgeToEdge;
import androidx.annotation.RequiresPermission;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;

import android.app.Activity;
import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ScrollView;
import android.widget.TextView;
import android.widget.Toast;

import com.example.bikenavunit.navlistener.NavigationData;
import com.example.bikenavunit.navlistener.NavigationListener;
import com.example.bikenavunit.navlistener.NavigationListenerEmitter;
import com.example.bikenavunit.navlistener.NavigationListenerEmitterKt;
import com.example.bikenavunit.navlistener.NavigationMode;
import com.example.bikenavunit.navlistener.NotificationCollectorMonitorService;
import com.example.bikenavunit.navlistener.NotificationListener;
import com.google.android.libraries.navigation.ListenableResultFuture;
import com.google.android.libraries.navigation.NavigationApi;
import com.google.android.libraries.navigation.Navigator;
import com.google.android.libraries.navigation.RoutingOptions;
import com.google.android.libraries.navigation.Waypoint;

import java.util.Arrays;
import java.util.Collections;
import java.util.Timer;
import java.util.TimerTask;

import androidx.navigation.NavController;

import com.example.bikenavunit.navlistener.NavigationListenerEmitterKt;
import com.example.bikenavunit.navlistener.NavigationListenerEmitterKt.*;

import com.example.bikenavunit.navlistener.NavigationListener;

public class MainActivity extends AppCompatActivity {
    private final static String TAG = "MAIN VIEW";

    private String mDeviceName;
    private String mDeviceAddress = "F4:5E:AB:7D:80:69";
    private BluetoothLEService mBluetoothLeService;
    private NotificationListener mNotificationListener;
    private boolean mConnected = false;
    private static MainActivity context;

    Navigator mNavigator;
    NavigationListener mNav;
    Timer timer = null;
    int tmpval = 0;

    public MainActivity() {
        context = this;
    }
    class SendTest extends TimerTask {
        @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
        public void run() {
            //calculate the new position of myBall
            if (null != mBluetoothLeService) {
                mBluetoothLeService.WriteMsg(String.format("--> %d <--", tmpval));
                tmpval++;
            }
        }
    }

    private final ServiceConnection mServiceConnection = new ServiceConnection() {

        @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
        @Override
        public void onServiceConnected(ComponentName componentName, IBinder service) {
            Log.i(TAG, "Component: " + componentName.toString());

            if (componentName.getClassName().contains("BluetoothLEService")) {
                mBluetoothLeService = ((BluetoothLEService.LocalBinder) service).getService();
                if (!mBluetoothLeService.initialize()) {
                    Log.e(TAG, "Unable to initialize Bluetooth");
                    finish();
                }

                Log.e(TAG, "mBluetoothLeService is okay");
                mBluetoothLeService.connect(mDeviceAddress);
            } else {
                mNotificationListener = ((NotificationListener.LocalBinder) service).getService();
                Log.e(TAG, "mNotificationListener is okay");
            }

            // Automatically connects to the device upon successful start-up initialization.
            //mBluetoothLeService.connect(mDeviceAddress);
        }

        @Override
        public void onServiceDisconnected(ComponentName componentName) {
            mBluetoothLeService = null;
        }
    };

    /*private final ServiceConnection mServiceConnection2 = new ServiceConnection() {

        @Override
        public void onServiceConnected(ComponentName componentName, IBinder service) {
            mNotificationListener = ((NotificationListener.LocalBinder) service).getService();

            Log.e(TAG, "mNotificationListener is okay");

            // Automatically connects to the device upon successful start-up initialization.
            //mBluetoothLeService.connect(mDeviceAddress);
        }

        @Override
        public void onServiceDisconnected(ComponentName componentName) {
            mBluetoothLeService = null;
        }
    };*/

    private final BroadcastReceiver mGattUpdateReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            final String action = intent.getAction();
            if (BluetoothLEService.ACTION_GATT_CONNECTED.equals(action)) {
                Log.e(TAG, "GATT connected");
                //TimerTask sendtask = new SendTest();
                //timer = new Timer();
                //timer.schedule(sendtask, 0, 1000);

            } else if (BluetoothLEService.ACTION_GATT_DISCONNECTED.equals(action)) {
                mConnected = false;
                invalidateOptionsMenu();
            } else if(BluetoothLEService.ACTION_GATT_SERVICES_DISCOVERED.equals(action)) {
                mConnected = true;
                Log.e(TAG, "Connected");
                invalidateOptionsMenu();
            }else if (BluetoothLEService.ACTION_DATA_AVAILABLE.equals(action)) {
                Log.e(TAG, "RECV DATA");
                String data = intent.getStringExtra(BluetoothLEService.EXTRA_DATA);
            } else {
                Log.e(TAG, "No clue");
            }
        }
    };

    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    @SuppressLint("InlinedApi")
    @Override
    protected void onResume() {
        super.onResume();
        registerReceiver(mGattUpdateReceiver, makeGattUpdateIntentFilter(), RECEIVER_EXPORTED);
        if (mBluetoothLeService != null) {
            final boolean result = mBluetoothLeService.connect(mDeviceAddress);
            Log.d(TAG, "Connect request result=" + result);
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        if (null != mGattUpdateReceiver) unregisterReceiver(mGattUpdateReceiver);
        //if (null != mServiceConnection) unbindService(mServiceConnection);

    }
    @RequiresPermission(Manifest.permission.BLUETOOTH_CONNECT)
    @Override
    protected void onDestroy() {
        super.onDestroy();
        //this.unregisterReceiver(mGattUpdateReceiver);
        //unbindService(mServiceConnection);
        if(mBluetoothLeService != null)
        {
            mBluetoothLeService.close();
            mBluetoothLeService = null;
        }
        Log.d(TAG, "We are in destroy");
    }


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        EdgeToEdge.enable(this);
        setContentView(R.layout.activity_main);
        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main), (v, insets) -> {
            Insets systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars());
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom);
            return insets;
        });
        checkPermission();
        //getNavigator();
        Intent serviceIntent = new Intent(context, NotificationCollectorMonitorService.class);
        //ContextCompat starts a background service on android < O automatically despite the method name
        ContextCompat.startForegroundService(context, serviceIntent);
        Intent i = new Intent(getApplicationContext(), NotificationListener.class);
        startService(i);
    }

    private static IntentFilter makeGattUpdateIntentFilter() {
        final IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(BluetoothLEService.ACTION_GATT_CONNECTED);
        intentFilter.addAction(BluetoothLEService.ACTION_GATT_DISCONNECTED);
        intentFilter.addAction(BluetoothLEService.ACTION_GATT_SERVICES_DISCOVERED);
        intentFilter.addAction(BluetoothLEService.ACTION_DATA_AVAILABLE);
        intentFilter.addAction(BluetoothDevice.ACTION_UUID);
        return intentFilter;
    }

    private void permissionsGranted() {

        /**
         do some stuff at startup, like creating some directories if not already present
         **/
        Intent gattServiceIntent = new Intent(this, BluetoothLEService.class);
        Log.d(TAG, "Try to bindService=" + bindService(gattServiceIntent, mServiceConnection, BIND_AUTO_CREATE));

        registerReceiver(mGattUpdateReceiver, makeGattUpdateIntentFilter(), RECEIVER_EXPORTED);



    }




    private void checkPermission() {
        if (
            ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED ||
            ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_ADMIN) != PackageManager.PERMISSION_GRANTED ||
            ContextCompat.checkSelfPermission(this, Manifest.permission.ACCESS_COARSE_LOCATION) != PackageManager.PERMISSION_GRANTED ||
            ContextCompat.checkSelfPermission(this, Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED ||
            ContextCompat.checkSelfPermission(this, Manifest.permission.BIND_NOTIFICATION_LISTENER_SERVICE) != PackageManager.PERMISSION_GRANTED ||
            ContextCompat.checkSelfPermission(this, Manifest.permission.FOREGROUND_SERVICE) != PackageManager.PERMISSION_GRANTED ||
            ContextCompat.checkSelfPermission(this, Manifest.permission.SYSTEM_ALERT_WINDOW) != PackageManager.PERMISSION_GRANTED ||
            ContextCompat.checkSelfPermission(this, Manifest.permission.ACCESS_BACKGROUND_LOCATION) != PackageManager.PERMISSION_GRANTED ||
            ContextCompat.checkSelfPermission(this, Manifest.permission.QUERY_ALL_PACKAGES) != PackageManager.PERMISSION_GRANTED ||
            ContextCompat.checkSelfPermission(this, "custom_perm_notifications_listener") != PackageManager.PERMISSION_GRANTED ||
            ContextCompat.checkSelfPermission(this, "custom_perm_notifications_service") != PackageManager.PERMISSION_GRANTED ||
            ContextCompat.checkSelfPermission(this, Manifest.permission.FOREGROUND_SERVICE_SPECIAL_USE) != PackageManager.PERMISSION_GRANTED

        ) {
            // do this if permisisons have not yet been granted
            Log.d("PERMISSION", "NOT granted");
            ActivityCompat.requestPermissions(this,
                    new String[]{
                            Manifest.permission.BLUETOOTH_ADMIN,
                            Manifest.permission.BLUETOOTH_ADMIN,
                            Manifest.permission.ACCESS_COARSE_LOCATION,
                            Manifest.permission.ACCESS_FINE_LOCATION,
                            Manifest.permission.BIND_NOTIFICATION_LISTENER_SERVICE,
                            Manifest.permission.FOREGROUND_SERVICE,
                            Manifest.permission.SYSTEM_ALERT_WINDOW,
                            Manifest.permission.ACCESS_BACKGROUND_LOCATION,
                            Manifest.permission.QUERY_ALL_PACKAGES,
                            "custom_perm_notifications_listener",
                            "custom_perm_notifications_service",
                            Manifest.permission.ACCESS_COARSE_LOCATION},
                    1);
        }else{
            // if the permissions have already been granted do the following
            Log.d("PERMISSION", "granted");
            permissionsGranted();
        }

        checkNotificationAccess();
    }

    // this is required when ActivityCompat.requestPermissions() is called
    @Override
    public void onRequestPermissionsResult(int requestCode, String permissions[], int[] grantResults) {
        Log.i("PERMISSION", Arrays.toString(permissions));
        Log.i("PERMISSION", Arrays.toString(grantResults));
        switch (requestCode) {
            case 1: {

                if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    // if permissions have been granted, do the following
                    permissionsGranted();
                } else {
                    Log.i("PERMISSION", "Not granted?");

                    // possibly prompt why permissions are required and try again
                }
                return;
            }

            // other 'case' lines to check for other
            // permissions this app might request.
        }
    }

    private Intent serviceIntent(String action) {
        return new Intent(getApplicationContext(), NavigationListenerEmitter.class).setAction(action);
    }

    protected void checkNotificationAccess() {
        startService(serviceIntent(NavigationListenerEmitterKt.CHECK_NOTIFICATIONS_ACCESS));
    }

    private void stopNavigation() {
        serviceIntent(NavigationListenerEmitterKt.STOP_NAVIGATION);
    }

    private void setServiceListenerIntent(PendingIntent pendingIntent) {
        startService(serviceIntent(NavigationListenerEmitterKt.SET_INTENT).putExtra(NavigationListenerEmitterKt.PENDING_INTENT, pendingIntent));
    }
    private void startServiceListener() {
        Log.d(TAG, "Start listener");
        //setServiceListenerIntent(createPendingResult(100, Intent(), 0));
    }

    public void getNavigator() {
        Log.d(TAG, "Starting navigationlistener");
        mNav = new NavigationListener();
        /*NavigationApi.getNavigator(this, new NavigationApi.NavigatorListener() {

            @Override
            public void onNavigatorReady(Navigator navigator) {
                Log.i("NAV", "Navigator ready.");
                mNavigator = navigator;
                boolean isNavInfoReceivingServiceRegistered =
                        mNavigator.registerServiceForNavUpdates(
                                getPackageName(), NavigationUnitService.class.getName(), 10);
                Log.e(TAG, "NAv registered: " + isNavInfoReceivingServiceRegistered);
                // Set the travel mode (DRIVING, WALKING, CYCLING, TWO_WHEELER, or TAXI).
                RoutingOptions mRoutingOptions = new RoutingOptions();
                mRoutingOptions.travelMode(RoutingOptions.TravelMode.CYCLING);

                // Navigate to a place, specified by Place ID.
                mNavigator.addRemainingTimeOrDistanceChangedListener(60, 100,
                        new Navigator.RemainingTimeOrDistanceChangedListener() {
                            @Override
                            public void onRemainingTimeOrDistanceChanged() {
                                Log.i("NAV", "onRemainingTimeOrDistanceChanged: Time or distance estimate"
                                                +   " has changed.");
                            }
                        });

                navigateToPlace("Oostburg", mRoutingOptions);

            }

            private void navigateToPlace(String placeId, RoutingOptions travelMode) {
                Waypoint destination;
                try {
                    destination = Waypoint.builder().setPlaceIdString(placeId).build();
                } catch (Waypoint.UnsupportedPlaceIdException e) {
                    Log.e("NASV", "Error starting navigation: Place ID is not supported.");
                    return;
                }

                // Create a future to await the result of the asynchronous navigator task.
                ListenableResultFuture<Navigator.RouteStatus> pendingRoute =
                        mNavigator.setDestination(destination, travelMode);

                // Define the action to perform when the SDK has determined the route.
                pendingRoute.setOnResultListener(
                        code -> {
                            switch (code) {
                                case OK:
                                    // Hide the toolbar to maximize the navigation UI.
                                    if (getActionBar() != null) {
                                        getActionBar().hide();
                                    }

                                    // Enable voice audio guidance (through the device speaker).
                                    mNavigator.setAudioGuidance(
                                            Navigator.AudioGuidance.VOICE_ALERTS_AND_GUIDANCE);

                                    // Simulate vehicle progress along the route for demo/debug builds.

                                    // Start turn-by-turn guidance along the current route.
                                    mNavigator.startGuidance();
                                    break;
                                // Handle error conditions returned by the navigator.
                                case NO_ROUTE_FOUND:
                                    Log.e("NAV", "Error starting navigation: No route found.");
                                    break;
                                case NETWORK_ERROR:
                                    Log.e("NAV", "Error starting navigation: Network error.");
                                    break;
                                case ROUTE_CANCELED:
                                    Log.e("NAV", "Error starting navigation: Route canceled.");
                                    break;
                                default:
                                    Log.e("NAV", "Error starting navigation: "
                                            + String.valueOf(code));
                            }
                        });
            }


            @Override
            public void onError(@NavigationApi.ErrorCode int errorCode) {
                switch (errorCode) {
                    case NavigationApi.ErrorCode.NOT_AUTHORIZED:
                        Log.e(TAG, "Error loading Navigation SDK: Your API key is "
                                + "invalid or not authorized to use the Navigation SDK.");
                        break;
                    case NavigationApi.ErrorCode.TERMS_NOT_ACCEPTED:
                        Log.e(TAG, "Error loading Navigation SDK: User did not accept "
                                + "the Navigation Terms of Use.");
                        break;
                    case NavigationApi.ErrorCode.NETWORK_ERROR:
                        Log.e(TAG, "Error loading Navigation SDK: Network error before nav.");
                        break;
                    case NavigationApi.ErrorCode.LOCATION_PERMISSION_MISSING:
                        Log.e(TAG, "Error loading Navigation SDK: Location permission "
                                + "is missing.");
                        break;
                    default:
                        Log.e(TAG, "Error loading Navigation SDK: " + errorCode);
                }
            }
        });*/
    }
}