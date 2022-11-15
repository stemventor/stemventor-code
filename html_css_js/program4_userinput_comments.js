function getUserCall() {
  // A function will often act upon values entered in the HTML form.
  // To get these values you have to use a library function
  // called getElementById which is called on the document.
  // This returns the element on which you can get its value attribute.

  const user_call = document.getElementById('user-call-input').value;
  
  // The console.log is a useful option to debug your code.
  // All browsers allow you to view the console by opening Developer Tools
  console.log(user_call);

  // You can also set the value of an element by using the innerHTML property of the element.
  // Set the your-call element to display to whatever was input in the text box and retrieved above.
  document.getElementById('your-call').innerHTML = user_call;
}